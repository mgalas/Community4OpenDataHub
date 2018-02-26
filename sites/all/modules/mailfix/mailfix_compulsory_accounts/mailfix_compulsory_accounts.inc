<?php
/**
 * @file
 * Definition of the Compulsory Accounts form
 */

function mailfix_compulsory_accounts_inc_debug($message = NULL, $type = 'status', $repeat = TRUE) {
  if (0)
    drupal_set_message($message, $type, $repeat);
}

function mailfix_compulsory_accounts_domain_from_email($email) {
  return drupal_substr($email, strpos($email, "@") + 1);
}

function mailfix_compulsory_accounts_make_account_name($pattern, $domain, $nice_name) {
  if ($nice_name)
    $subdomain = $nice_name;
  else
    $subdomain = ucfirst(preg_replace("/\..*/", "", $domain));

  $patterns = array();
  $patterns[] = '/!subdomain/';
  $patterns[] = '/!domain/';

  $replacements = array();
  $replacements[] = $subdomain;
  $replacements[] = $domain;

  $result = preg_replace($patterns, $replacements, $pattern);
  return $result;
}

function mailfix_compulsory_accounts_get_existing_matching_pattern($email, $account, &$existing) {
  $patterns = array();
  $patterns[] = '/!subdomain/';
  $patterns[] = '/!domain/';

  $replacements = array();
  $replacements[] = '%';
  $replacements[] = '%';
  $account_text = preg_replace($patterns, $replacements, $account);

  $sql = "SELECT uid, mail, name FROM {users} WHERE mail LIKE '%s@%' OR name like '%s'";
  $result = db_query($sql, $email, $account_text);

  $exempt_sql = "SELECT domain_name AS name FROM {mailfix_domains} WHERE exempt = 1";
  $exempt_results = db_query($exempt_sql);

  $exemptions = array();
  foreach ($exempt_results as $exemption) {
    $exemptions[$exemption->name] = 1;
  }

  foreach ($result as $account) {
    $domain = mailfix_compulsory_accounts_domain_from_email($account->mail);
    if ($exemptions[$domain])
      continue;

    $existing[$domain]['existing'][] = array(
      'uid' => $account->uid,
      'mail' => $account->mail,
      'account' => $account->name);
  }
}

function mailfix_compulsory_accounts_apply_rule($email, $account) {

  // Start by getting the list of domains we need to work on.
  $sql = "SELECT domain_name AS name, domain_id AS id, nice_name from {mailfix_domains} WHERE exempt = 0 ORDER BY name";
  $result = db_query($sql);

  $existing = array();

  foreach ($result as $domain) {
    $existing[$domain->name]['id'] = $domain->id;
    $existing[$domain->name]['nice_name'] = $domain->nice_name;
    $existing[$domain->name]['exempt'] = $domain->exempt;
  }

  // Now find out if there are any pre-existing email addresses or account names that match.
  mailfix_compulsory_accounts_get_existing_matching_pattern($email, $account, &$existing);

  // We now have all the info we need to add/update accounts.

  $added = 0;
  $failed_adds = 0;
  $email_changed = 0;
  $account_name_changed = 0;
  $errors = '';

  foreach ($existing as $domain => &$state) {
    $new_email = $email . '@' . $domain;
    $new_account_name = mailfix_compulsory_accounts_make_account_name($account, $domain, $state['nice_name']);

    if (empty($state['existing'])) {
      mailfix_compulsory_accounts_inc_debug("Add account in domain $domain");

      // We rely on genpass to create a password.
      $new_user = array(
        'name' => $new_account_name,
        'mail' => $new_email,
        'status' => 1,
        'init' => $new_email,
      );

      $result = user_save(null, $new_user);

      if ($result) {
        drupal_set_message(t("Successfully created user %user.", array('%user' => $new_account_name)));
        $added++;
      }
      else {
        drupal_set_message(t("Failed to create user %user.", array('%user' => $new_account_name)));
        $failed_adds++;
      }
      continue;
    }

    if (count($state['existing']) == 1) {
      if ($state['existing'][0]['mail'] != $new_email) {
        drupal_set_message(t('Changing address %mail associated with account %account', array(
          '%mail' => $state['existing'][0]['mail'],
          '%account' => $state['existing'][0]['account'])));
        $existing_user = user_load($state['existing'][0]['uid']);
        $existing_user->mail = $new_email;
        $result = user_save((object) array('uid' => $existing_user->uid), (array) $existing_user);
        $email_changed++;
      }
      elseif ($state['existing'][0]['account'] != $new_account_name) {
        drupal_set_message(t('Changing account name from %account for address %mail.', array(
          '%mail' => $state['existing'][0]['mail'],
          '%account' => $state['existing'][0]['account'])));
        $existing_user = user_load($state['existing'][0]['uid']);
        $existing_user->name = $new_account_name;
        $result = user_save((object) array('uid' => $existing_user->uid), (array) $existing_user);
        $account_name_changed++;
      }
      else {
        drupal_set_message(t('Nothing to do for account %account.', array(
          '%account' => $state['existing'][0]['account'])));
      }
    }
    else {
      $msg = "Domain $domain has multiple existing accounts that overlap with your settings."
        . "You will need to decide what to do with ";
      $num = count($state['existing']);
      $msg .= $num . " accounts named ";
      for ($i = 0; $i < $num - 2; $i++)
        $msg .= $state['existing'][$i]['account'] . ' (' . $state['existing'][$i]['mail'] . '), ';
      if ($num > 1)
       $msg .= $state['existing'][$num - 2]['account'] . ' (' . $state['existing'][$num -2]['mail'] . ') and ';
      $msg .= $state['existing'][$num - 1]['account'] . ' (' . $state['existing'][$num - 1]['mail'] . ').';

      drupal_set_message($msg);
      $errors .= $msg;
    }
  }

  $result = "";
  if ($added)
    $result .= format_plural($added, '1 account was added. ', '@count accounts were added. ');
  if ($failed_adds)
    $result .= format_plural($failed_adds, '1 account could not be added. ', '@count accounts could not be added. ');
  if ($email_changed)
    $result .= format_plural($email_changed, 'The email address of one account was changed. ',
                               'The email addresses of @count accounts were changed. ');
  if ($account_name_changed)
    $result .= format_plural($account_name_changed, 'The name of one account was changed. ',
                               'The name of @count accounts were changed. ');
  if ($errors)
    $result .= $errors;

  if (!$result)
    $result = "No changes were made.";

  return trim($result);
}

/**
 * Menu callback. Displays compulsory accounts configuration form.
 */
function mailfix_compulsory_accounts_overview() {
  // Render compulsory accounts list.
  $sql = 'SELECT aid, email, username FROM {mailfix_compulsory_accounts} ORDER BY email';
  $result = db_query($sql);
  $header = array(
    t('Email'),
    t('Account Name'),
  );
  $rows = array();

  foreach ($result as $account) {
    $row = array(
      $account->email,
      $account->username,
    );
    // Set the edit column
    $row[] = array('data' => l(t('edit'), 'admin/settings/mailfix_compulsory_accounts/edit/' . $account->aid), ) ;
    // Set the delete column
    $row[] = array('data' => l(t('delete'), 'admin/settings/mailfix_compulsory_accounts/delete/' . $account->aid), ) ;
    $rows[] = $row;
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No compulsory accounts were found.'), 'colspan' => 5, 'class' => 'message'));
  }

  $output = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => $header,
  );
  return $output;
}  // mailfix_compulsory_accounts_overview

/**
 * Menu callback: Generate a form to add a new compulsory address
 *
 * @see mailfix_compulsory_accounts_form_validate()
 * @see mailfix_compulsory_accounts_form_submit()
 */
function mailfix_compulsory_accounts_form(&$form_state, $arg = NULL) {
  $form['#redirect'] = 'admin/settings/mailfix_compulsory_accounts';
  $form['email'] = array(
    '#type' => 'textfield',
    '#title' => t('Email Address'),
    '#description' => t('Enter the address you want all domains to have. For example, if you want all domains to have a news@... address, type \'news\' (without the quotes).'),
    '#required' => TRUE,
  );
  $form['account'] = array(
    '#type' => 'textfield',
    '#title' => t('Account Name'),
    '#description' => t('Enter the account name format to be used for each domain. If, for example, you want \'Melbourne News\' for the domain melbourne.example.com, just type \'News\'. If you want some other format, put \'!subdomain\' where \'Melbourne\' should be placed, or \'!domain\' for the whole domain as is.'),
    '#required' => TRUE,
  );
  $form['submit'] = array('#type' => 'submit',
    '#value' => t('Add compulsory account'),
  );
  return $form;
}  // mailfix_compulsory_accounts_form

/**
 * Implementation of hook_validate().
 * 
 * Validate mailfix_compulsory_accounts_form form submissions for valid domain names.
 */
function mailfix_compulsory_accounts_form_validate($form, &$form_state) {
  if (!preg_match('/^[\da-z0-9\-_]+$/i', $form_state['values']['email'])) {
    form_set_error('email', t('The email address given contains one or more illegal characters. You only need to enter the part prior to the \'@\', using a-z, -, _ and digits.'));
  }
  if (!preg_match('/^(([\da-zA-Z0-9\ ]+)|!(sub)?domain)+$/i', $form_state['values']['account'])) {
    form_set_error('account', t('The Account Name given contains one or more illegal characters.'));
  }
  if (db_result(db_query("SELECT email FROM {mailfix_compulsory_accounts} WHERE email = '%s'", 
      $form_state['values']['email']))) {
    form_set_error('title', t('The specified email address is already compulsory.'));
  }
}  // mailfix_compulsory_accounts_form_validate

/**
 * Process mailfix_compulsory_accounts_form form submissions to add new domain.
 */
function mailfix_compulsory_accounts_form_submit($form, &$form_state) {

  // Figure out what we need to do.
  $outcome = mailfix_compulsory_accounts_apply_rule($form_state['values']['email'],
        $form_state['values']['account']);

  db_query("INSERT INTO {mailfix_compulsory_accounts} (email, username) VALUES ('%s', '%s')",
    $form_state['values']['email'], $form_state['values']['account']);

  watchdog(
    'mailfix_compulsory_accounts', 
    'Compulsory account %name created. Accounts are of format \"%account\". %outcome', 
    array(
      '%name' => $form_state['values']['email'], 
      '%account' => $form_state['values']['account'],
      '%outcome' => $outcome,
    ),
    WATCHDOG_NOTICE, 
    l(t('add'), 'admin/settings/mailfix_compulsory_accounts/add')
  );
} // mailfix_compulsory_accounts_form_submit
 
/**
 * Menu callback: Generate a form to edit an existing domain
 *
 */
function mailfix_compulsory_accounts_edit_form(&$form_state, $aid) {
  $obj = db_query('SELECT aid, email, username FROM {mailfix_compulsory_accounts} mca WHERE mca.aid = %d', $aid);

  if (!$obj) {
    drupal_not_found();
    return;
  }

  $form = mailfix_compulsory_accounts_form($form_state, NULL);
  $form['aid'] = array('#type' => 'value', '#value' => $aid);
  $form['email']['#default_value'] = $obj->email;
  $form['account']['#default_value'] = $obj->username;
  $form['submit']['#value'] = t('Update compulsory account');

  return $form;
}  // mailfix_edit_domain_form

/**
 * Implementation of hook_validate().
 * 
 * Validate mailfix_edit_domain_form form submissions for valid quota.
 */
function mailfix_compulsory_accounts_edit_form_validate($form, &$form_state) {
  if (!preg_match('/^[\da-z0-9\-_]+$/i', $form_state['values']['email'])) {
    form_set_error('email', t('The email address given contains one or more illegal characters. You only need to enter the part prior to the \'@\', using a-z, -, _ and digits.'));
  }
  if (!preg_match('/^(([\da-zA-Z0-9\ ]+)|!(sub)?domain)+$/i', $form_state['values']['account'])) {
    form_set_error('account', t('The Account Name given contains one or more illegal characters.'));
  }
}  // mailfix_edit_domain_form_validate

/**
 * Process mailfix_edit_domain_form form submissions to update existing domain.
 */
function mailfix_compulsory_accounts_edit_form_submit($form, &$form_state) {
  // Figure out what we need to do.
  $old = db_query('SELECT aid, email, username FROM {mailfix_compulsory_accounts} mca WHERE mca.aid = %d', $form_state['values']['aid']);

  $outcome = mailfix_compulsory_accounts_apply_rule($form_state['values']['email'],
        $form_state['values']['account']);

  db_query("UPDATE {mailfix_compulsory_accounts} SET email='%s', username='%s' WHERE aid=%d", array(
    $form_state['values']['email'], $form_state['values']['account'], $form_state['values']['aid']));

  watchdog(
    'mailfix_compulsory_accounts', 
    'Compulsory account %aid updated. Email is now %name. Accounts are of format \"%account\". %outcome', 
    array(
      '%aid' => $form_state['values']['aid'],
      '%name' => $form_state['values']['email'], 
      '%account' => $form_state['values']['account'],
      '%outcome' => $outcome,
    ),
    WATCHDOG_NOTICE, 
    l(t('add'), 'admin/settings/mailfix_compulsory_accounts/add')
  );
} // mailfix_edit_domain_form_submit

/**
 * Menu callback. Deletes specified domain.
 */
function mailfix_compulsory_accounts_delete_form(&$form_state, $account_id) {
  $account = db_query('SELECT aid, email, username FROM {mailfix_compulsory_accounts} mca WHERE mca.aid = %d', $account_id);
  if (!$account) {
    drupal_not_found();
    return;
  }
  $form['aid'] = array('#type' => 'value', '#value' => $account_id);
  $form['email'] = array('#type' => 'value', '#value' => $account[0]->email);
  $form['account'] = array('#type' => 'value', '#value' => $account[0]->username);
  return confirm_form(
    $form,
    t('Are you sure you want to stop enforcing the compulsory account <em>%email</em>?', array('%email' => $account[0]->email)), 
    'admin/settings/mailfix_compulsory_accounts/list',  // if user denies the action this is the landing path
    t('All existing accounts will be kept, but users will be able to modify accounts.'),
    t('Delete'), 
    t('Cancel')
  );
}

/**
 * Form submit callback; confirms deleting a domain.
 */
function mailfix_compulsory_accounts_delete_form_submit($form, &$form_state) {
  db_query('DELETE FROM {mailfix_compulsory_accounts} WHERE aid = %d',
    $form_state['values']['aid']);

  drupal_set_message(t('Mailfix Compulsory Account %name has been deleted.', 
    array(
      '%name' => $form_state['values']['aid'],
    )));
  watchdog(
    'mailfix', 
    'Mailfix Compulsory Account %name has been deleted.', 
    array(
      '%name' => $form_state['values']['aid'],
    ), 
    WATCHDOG_NOTICE, 
    l(t('view'), 'admin/settings/mailfix_compulsory_accounts')
  );

  $form_state['redirect'] = 'admin/settings/mailfix_compulsory_accounts';
  return;
}  // mailfix_compulsory_accounts_delete_confirm

