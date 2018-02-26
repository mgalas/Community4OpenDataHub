<?php
/**
 * @file
 * Included by mailfix.module to perform Mailfix domain management operations.
 */
 
/**
 * Form submit callback; confirms deleting a domain.
 */
function mailfix_domain_delete_submit($form, &$form_state, $domain_id = NULL) {
  $subquery = db_select('mailfix_users', 'm');
  $subquery->addExpression('COUNT(uid)');
  $subquery->condition('domain_id', $form_state['values']['domain_id'], '=');
  $result = $subquery->execute()
    ->fetchField();
  db_delete('mailfix_users')
    ->condition('domain_id', $form_state['values']['domain_id'])
    ->execute();
  db_delete('mailfix_domains')
    ->condition('domain_id', $form_state['values']['domain_id'])
    ->execute();

  drupal_set_message(t('Domain %domain_name has been deleted. %domain_users.', 
    array(
      '%domain_name' => $form_state['values']['domain_name'],
      '%domain_users' => format_plural($result, '1 Mailfix user profile was deleted', '@count Mailfix user profiles were deleted'),
    )));
  watchdog(
    'mailfix', 
    'Domain %domain_name has been deleted. %domain_users.', 
    array(
      '%domain_name' => $form_state['values']['domain_name'],
      '%domain_users' => format_plural($result, '1 Mailfix user profile was deleted', '@count Mailfix user profiles were deleted'),
    ), 
    WATCHDOG_NOTICE, 
    l(t('view'), 'admin/config/system/mailfix')
  );

  $form_state['redirect'] = 'admin/config/system/mailfix';
  return;
}  // mailfix_domain_delete_confirm

/**
 * Menu callback. Deletes specified domain.
 */
function mailfix_domain_delete($form, &$form_state, $domain_id = NULL) {
  if (is_null($domain_id)) {
    drupal_goto('admin/config/system/mailfix');
    return;
  }
  $domain = db_query('SELECT domain_name FROM {mailfix_domains} d WHERE d.domain_id = :domain_id',
	array(':domain_id' => $domain_id))->fetchField();
  if (!$domain) {
    drupal_goto('admin/config/system/mailfix');
    return;
  }
  $form['domain_id'] = array('#type' => 'value', '#value' => $domain_id);
  $form['domain_name'] = array('#type' => 'value', '#value' => $domain);
  return confirm_form(
    $form,
    t('Are you sure you want to delete the domain <em>%domain</em>?', array('%domain' => $domain)), 
    'admin/config/system/mailfix/list',  // if user denies the action this is the landing path
    t('This action cannot be undone. All exising email addresses belonging to this domain will no longer be managed by this mail server.'),
    t('Delete'), 
    t('Cancel')
  );
}

/**
 * Menu callback: Generate a form to add an existing domain
 *
 * @see mailfix_add_domain_form_validate()
 * @see mailfix_add_domain_form_submit()
 */
function mailfix_add_domain_form($form_state, $arg = NULL) {
  $form['#redirect'] = 'admin/config/system/mailfix';
  $form['domain_name'] = array(
    '#type' => 'textfield',
    '#title' => t('Domain name'),
    '#description' => t('Enter a valid domain name for which this Drupal server is intended to manage mail, e.g. example.com.'),
    '#required' => TRUE,
    '#weight' => 1,
  );
  $form['default_quota'] = array(
    '#type' => 'textfield',
    '#default_value' => 104857600, // default value for quota 100MB = 1024 * 1024 * 100 = 104857600 bytes
    '#title' => t('Default storage quota'),
    '#description' => t('Enter the default mailbox storage quota (in bytes). Default value of 104857600 corresponds to 100MB. For unlimited quota set this value to 0.'),
    '#required' => TRUE,
    '#weight' => 2,
  );
  $form['submit'] = array('#type' => 'submit',
    '#value' => t('Add domain'),
    '#weight' => 1000,
  );
  return $form;
}  // mailfix_add_domain_form

/**
 * Implementation of hook_validate().
 * 
 * Validate mailfix_add_domain_form form submissions for valid domain names.
 */
function mailfix_add_domain_form_validate($form, &$form_state) {
  if (!preg_match('/^([\da-zA-Z-]{1,}\.){1,}[\da-zA-Z-]+$/i', $form_state['values']['domain_name'])) {
    form_set_error('domain name', t('The specified domain name contains one or more illegal characters. Spaces or any other special characters except dash (-) and underscore (_) are not allowed.'));
  }
  if (isset($form_state['values']['quota']) && $form_state['values']['quota'] < 0) {
    form_set_error('mail storage quota', t('The mail storage quota must be a positive integer value.'));
  }
  $result = db_query("SELECT count(d.domain_id) FROM {mailfix_domains} d WHERE domain_name = ':name'", 
	  array(':name' => $form_state['values']['domain_name']))->fetchField();
  if ($result) {
    form_set_error('title', t('The specified domain name is already registered.'));
  }
}  // mailfix_add_domain_form_validate

/**
 * Process mailfix_add_domain_form form submissions to add new domain.
 */
function mailfix_add_domain_form_submit($form, &$form_state) {
  $domain = db_select('mailfix_domains', 'm')
    ->fields('m')
    ->condition('domain_name', $form_state['values']['domain_name'], '=')
    ->execute()
    ->fetchAssoc();

  if (!empty($domain)) {
    drupal_set_message(t('Domain :name is already managed by Mailfix.', array(':name' => $form_state['values']['domain_name'])));
    return;
  }

  $id = db_insert('mailfix_domains')
	  ->fields(array(
		  'domain_name' => $form_state['values']['domain_name'],
		  'default_quota' => $form_state['values']['default_quota']
	  ))
	  ->execute();
  $domain = db_select('mailfix_domains', 'm')
    ->fields('m')
    ->condition('domain_name', $form_state['values']['domain_name'], '=')
    ->execute()
    ->fetchAssoc();

  if ($domain['domain_id'] > 0) {
    // Clean out any wrongly associated uid, as this field is a unique index
    $subquery = db_select('users', 'u')
      ->fields('u', array('uid'))
      ->condition('mail', '%@:' . $domain['domain_name'], 'LIKE');
    db_delete('mailfix_users')
      ->condition('uid', $subquery, 'IN')
      ->execute();
    
    // Populate mailfix_users
    $subquery = db_select('users', 'u');
    $subquery->addField('u', 'uid');
    $subquery->addExpression($domain['domain_id'], 'domain_id');
    $subquery->condition('u.mail',  '%@' . $domain['domain_name'], 'LIKE');
    db_insert('mailfix_users')
	    ->from($subquery)
	    ->execute();
  }
  // How many Mailfix profiles were created?
  $subquery = db_select('mailfix_users', 'm');
  $subquery->addExpression('COUNT(uid)');
  $subquery->condition('domain_id', $domain['domain_id'], '=');
  $result = $subquery->execute()
    ->fetchField();
  drupal_set_message(t('Domain name %domain_name created. %domain_users with default settings.', array(
      '%domain_name' => $form_state['values']['domain_name'], 
      '%domain_users' => format_plural($result, '1 user was found and configured', '@count users were found and configured'),
    )
  ));
  watchdog(
    'mailfix', 
    'Domain name %domain_name created. %domain_users with default settings.', 
    array(
      '%domain_name' => $form_state['values']['domain_name'], 
      '%domain_users' => format_plural($result, '1 user was found and configured', '@count users were found and configured'),
    ),
    WATCHDOG_NOTICE, 
    l(t('view'), 'admin/config/system/mailfix')
  );
} // mailfix_add_domain_form_submit
 
/**
 * Menu callback: Generate a form to edit an existing domain
 *
 * @see mailfix_edit_domain_form_validate()
 * @see mailfix_edit_domain_form_submit()
 */
function mailfix_edit_domain_form($form, &$form_state, $domain_id = NULL) {
  if (is_null($domain_id)) {
    drupal_goto('admin/config/system/mailfix');
    return;
  }

  $domain = db_select('mailfix_domains', 'd')
    ->fields('d', array('domain_id', 'domain_name', 'default_quota'))
    ->condition('domain_id', $domain_id, '=')
    ->execute()
    ->fetchAssoc();
  
  if (!$domain) {
    drupal_goto('admin/config/system/mailfix');
    return;
  }
  $content['domain_id'] = array('#type' => 'value', '#value' => $domain_id);
  $content['domain_name'] = array('#type' => 'value', '#value' => $domain['domain_name']);
  $content['#redirect'] = 'admin/config/system/mailfix';
  $content['domain_label'] = array(
    '#type' => 'item',
    '#title' => t('Domain name'),
    '#description' => t('This is the registered domain name for which this Drupal server is intended to manage mail.'),
    '#value' => $domain['domain_name'],
  );
  $content['default_quota'] = array(
    '#type' => 'textfield',
    '#default_value' => $domain['default_quota'],
    '#title' => t('Default storage quota'),
    '#description' => t('This is the default mailbox storage quota (in bytes). Default value of 104857600 corresponds to 100MB. For unlimited quota set this value to 0.'),
    '#required' => TRUE,
  );
  $actions = array('submit' => array('#type' => 'submit',
    '#value' => t('Update domain'),
    '#weight' => 1000,
  ));
  return array(
    '#type' => 'form',
    'content' => $content,
    'actions' => $actions,
  );
}  // mailfix_edit_domain_form

/**
 * Implementation of hook_validate().
 * 
 * Validate mailfix_edit_domain_form form submissions for valid quota.
 */
function mailfix_edit_domain_form_validate($form, &$form_state) {
  if (parse_size($form_state['values']['default_quota']) < 0) {
    form_set_error('mail storage quota', t('The mail storage quota must be a positive integer value.'));
  }
}  // mailfix_edit_domain_form_validate

/**
 * Process mailfix_edit_domain_form form submissions to update existing domain.
 */
function mailfix_edit_domain_form_submit($form, &$form_state) {
  db_query(
    "UPDATE {mailfix_domains} SET default_quota = :quota WHERE domain_id = :domain_id",
    array(':quota' => parse_size($form_state['values']['default_quota']),
          ':domain_id' => $form_state['values']['domain_id']));
  $sql = "SELECT * FROM {mailfix_domains} WHERE domain_id = :domain_id ";
  $domain = db_query($sql, array(':domain_id' => $form_state['values']['domain_name']));
  // Alert user of update operation.
  drupal_set_message(t('Default quota for domain %domain_name updated.', array(
      '%domain_name' => $form_state['values']['domain_name'], 
    )
  ));
  drupal_goto('admin/config/system/mailfix');
} // mailfix_edit_domain_form_submit

/**
 * Menu callback. Displays domains configuration form.
 */
function mailfix_domains_overview() {
  // Render domains list.
  $sql = 'SELECT d.domain_id, d.domain_name, d.default_quota, count(u.uid) AS domain_users '
    . 'FROM {mailfix_domains} d LEFT JOIN {mailfix_users} u ON d.domain_id = u.domain_id '
    . 'GROUP BY  d.domain_id, d.domain_name, d.default_quota '
    . 'ORDER BY d.domain_name';
  $result = db_query($sql);
  $header = array(
    t('Domains'), 
    t('Current users'), 
    t('Default quota'), 
//    t('Operations'),
    array('data' => t('Operations'), 'colspan' => 2),
  );
  $rows = array();

  foreach ($result as $domain) {
    $row = array(
      $domain->domain_name, 
      $domain->domain_users, 
      format_size($domain->default_quota), 
//      l(t('delete'), 'admin/config/system/mailfix/delete/'. $domain->domain_id),
    );
    // Set the edit column
    $row[] = array('data' => l(t('edit'), 'admin/config/system/mailfix/edit/' . $domain->domain_id), ) ;
    // Set the delete column
    $row[] = array('data' => l(t('delete'), 'admin/config/system/mailfix/delete/' . $domain->domain_id), ) ;
    $rows[] = $row;
  }

  if (empty($rows)) {
    $rows[] = array(array('data' => t('No registered mail domains were found.'), 'colspan' => 5, 'class' => 'message'));
  }

  $output = array(
    '#theme' => 'table',
    '#rows' => $rows,
    '#header' => $header,
  );
  return $output;
}  // mailfix_domains_overview

