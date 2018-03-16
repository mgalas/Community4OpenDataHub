<?php

/**
 * @file
 * Register this plugin via the Views 2 API.
 *
 */

function mailfix_views_data() {
  // Define the base group of this table. Fields that don't
  // have a group defined will go into this field by default.
  $data['mailfix_users']['table']['group']  = t('Mailfix User');

  //joins
  $data['mailfix_users']['table']['join']['users'] = array(
      'left_field' => 'uid',
      'field' => 'uid',
  );

  // ----------------------------------------------------------------
  // Fields

  // forwarding address(es)
  $data['mailfix_users']['forward'] = array(
    'title' => t('Forwarding'),
    'help' => t('Where email is forwarded (if anywhere).'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  $data['mailfix_domains']['table']['group']  = t('Mailfix User');

  $data['mailfix_domains']['table']['join']['users'] = array(
      'left_table' => 'mailfix_users',
      'left_field' => 'domain_id',
      'field' => 'domain_id',
  );

  // Domain Name
  $data['mailfix_domains']['domain_name'] = array(
    'title' => t('Domain name'),
    'help' => t('The domain name.'),
    'field' => array(
      'handler' => 'views_handler_field',
      'click sortable' => TRUE,
    ),
    'filter' => array(
      'handler' => 'views_handler_filter_string',
    ),
    'sort' => array(
      'handler' => 'views_handler_sort',
    ),
    'argument' => array(
      'handler' => 'views_handler_argument_string',
    ),
  );

  if (module_exists('mailfix_admin')) {
    $data['mailfix_domain_admins']['table']['group']  = t('Mailfix User');

    //joins
    $data['mailfix_domain_admins']['table']['join']['users'] = array(
        'left_table' => 'mailfix_users',
        'left_field' => 'domain_id',
        'field' => 'domain_id',
    );

    // Has Mailfix Domain Edit Access
    $data['mailfix_domain_admins']['mailfix_domain_admin_access'] = array(
      'title' => t('Mailfix Domain Admin Permission'),
      'help' => t('Whether the user has Mailfix Domain Admin permission'),
      'real field' => 'uid',
      'field' => array(
        'handler' => 'mailfix_views_handler_field_mailfix_domain_admin_access',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'mailfix_views_handler_filter_mailfix_domain_admin_access',
        'label' => t('Mailfix Domain Admin'),
        'type' => 'yes-no',
      ),
    );

    $data['mailfix_domain_admins_users']['table']['group']  = t('Mailfix User');

    //joins
    $data['mailfix_domain_admins_users']['table']['join']['users'] = array(
        'left_table' => 'users',
        'left_field' => 'uid',
        'field' => 'tuid',
    );

    // Has Mailfix Explicit Edit Access
    $data['mailfix_domain_admins_users']['mailfix_explicit_admin_access'] = array(
      'title' => t('Mailfix User Admin Permission'),
      'help' => t('Whether the user has explicit Mailfix Admin permission'),
      'real field' => 'uid',
      'field' => array(
        'handler' => 'mailfix_views_handler_field_mailfix_user_admin_access',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'mailfix_views_handler_filter_mailfix_user_admin_access',
        'label' => t('Mailfix User Admin'),
        'type' => 'yes-no',
      ),
    );

    // Has Mailfix Any Edit Access
    $data['mailfix_domain_admins']['mailfix_any_admin_access'] = array(
      'title' => t('Mailfix Any Admin Permission'),
      'help' => t('Whether the user has any Mailfix Admin permission'),
      'real field' => 'uid',
      'field' => array(
        'handler' => 'mailfix_views_handler_field_mailfix_any_admin_access',
        'click sortable' => TRUE,
      ),
      'filter' => array(
        'handler' => 'mailfix_views_handler_filter_mailfix_any_admin_access',
        'label' => t('Mailfix Admin (Domain or User)'),
        'type' => 'yes-no',
      ),
    );

  };

  return $data;
}

function mailfix_views_data_alter(&$data) {
  $data['users']['table']['join']['mailfix'] = array(
    'left_table' => 'mailfix_users',
    'left_field' => 'uid',
    'field' => 'uid',
  );

  if (module_exists('mailfix_admin')) {
    $data['mailfix_domain_admins']['mailfix_domain_admin_access']['field']['handler'] =
          'mailfix_views_handler_field_mailfix_domain_admin_access';
  }
}

function mailfix_views_views_handlers() {

  if (module_exists('mailfix_admin')) {
    return array(
      'info' => array(
        'path' => drupal_get_path('module', 'mailfix_views') . '/includes',
      ),
      'handlers' => array(
        // field handlers
        'mailfix_views_handler_field_mailfix_domain_admin_access' => array(
          'parent' => 'views_handler_field_boolean',
        ),
        'mailfix_views_handler_field_mailfix_user_admin_access' => array(
          'parent' => 'views_handler_field_boolean',
        ),
        'mailfix_views_handler_field_mailfix_any_admin_access' => array(
          'parent' => 'views_handler_field_boolean',
        ),
        // filter handlers
        'mailfix_views_handler_filter_mailfix_domain_admin_access' => array(
          'parent' => 'views_handler_filter_boolean_operator',
        ),
        'mailfix_views_handler_filter_mailfix_user_admin_access' => array(
          'parent' => 'views_handler_filter_boolean_operator',
        ),
        'mailfix_views_handler_filter_mailfix_any_admin_access' => array(
          'parent' => 'views_handler_filter_boolean_operator',
        ),
      )
    );
  };
}
