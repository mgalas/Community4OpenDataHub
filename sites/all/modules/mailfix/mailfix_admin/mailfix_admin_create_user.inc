<?php

/**
 * @file
 * Enables a Mailfix domain admin to create users in their domain
 */

function mailfix_admin_create_user_form($form, &$form_state) {
  $form = array();

  $form['account']['name'] = array('#type' => 'textfield',
    '#title' => t('Username'),
    '#default_value' => '',
    '#maxlength' => USERNAME_MAX_LENGTH,
    '#size' => NULL,
    '#description' => t('Spaces are allowed; punctuation is not allowed except for periods, hyphens, and underscores.'),
    '#required' => TRUE,
  );
  $form['account']['mail'] = array('#type' => 'textfield',
    '#title' => t('E-mail address'),
    '#default_value' => '',
    '#maxlength' => EMAIL_MAX_LENGTH,
    '#size' => NULL,
    '#description' => t('A valid e-mail address that you wish to create, in a domain you administer.'),
    '#required' => TRUE,
  );
  $form['submit'] = array('#type' => 'submit',
    '#value' => t('Add account'),
  );
  return $form;
}

function mailfix_admin_create_user_form_validate($form, &$form_state) {
  global $user;

  mailfix_admin_debug('mailfix_admin_create_user_validate');

  $form['#user_category'] = 'register';
  $form['#user'] = new stdClass();
  $form['#user']->uid = 0;

  if (user_account_form_validate($form, $form_state))
    return;

  if (user_access('administer users')) {
    mailfix_admin_debug('has mailfix_admin_create_user_validate');
    return;
  }

  // If the user can only create Mailfix domain users, check they're doing that.
  $domain_id = mailfix_domain_id_from_email($form_state['values']['mail']);

  if (!$domain_id) { // Not a Mailfix domain
    form_set_error('mail', 'This is not a domain managed by Mailfix.');
    return;
  }

  $result = db_query("SELECT md.domain_id FROM {mailfix_domain_admins} md WHERE md.uid = :uid AND md.domain_id = :domain ",
        array(':uid' => $user->uid, ':domain' => $domain_id))
    ->fetchField();

  if (!$result) {
    form_set_error('mail', 'You do not have permission to administer this domain.');
    return;
  }
}

function mailfix_admin_create_user_form_submit($form, &$form_state) {
  mailfix_admin_debug('mailfix_admin_create_user_submit');

  // We rely on genpass to create a password.
  $new_user = array(
    'name' => $form_state['values']['name'],
    'pass' => user_password(),
    'init' => $form_state['values']['name'],
    'status' => 1,
    'mail' => $form_state['values']['mail'],
  );

  $result = user_save('', $new_user);

  if (!$result) {
    drupal_set_message(t('Failed to create the user.'));
    return;
  }

  // Take the admin to the user's edit page.
  drupal_goto('user/' . $result->uid . '/edit');
}
