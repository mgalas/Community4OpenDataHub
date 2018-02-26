<?php

/**
 * @file
 * Provides editor view node access filter
 */
 
class mailfix_views_handler_filter_mailfix_any_admin_access extends views_handler_filter_boolean_operator {
  function can_expose() {
    return TRUE;
  }

  function operators() {
    return array(
      'in' => array(
        'title' => t('User is Mailfix Any Admin'),
        'short' => t('mailfix_any_admin'),
        'method' => 'op_simple',
        'values' => 1,
      ),
    );
  } 

  function query() {
    global $user;

    if ($user->uid != 1 && !user_access('administer users')) {
      $table = $this->ensure_my_table();
      $join = new views_join();
      $join->construct('mailfix_users', $this->table_alias, 'uid', 'uid');
      $this->query->ensure_table('mailfix_users', $this->relationship, $join);

      $join = new views_join();
      $join->construct('mailfix_domain_admins_users', 'users', 'uid', 'tuid');
      $this->query->ensure_table('mailfix_domain_admins_users', $this->relationship, $join);

      $where = "mailfix_domain_admins.uid ";

      if (empty($this->value)) {
        $where .= '<> ***CURRENT_USER***';
        $where = '(' . $where . " OR mailfix_domain_admins.uid IS NULL)";
      }
      else {
        $where .= '= ***CURRENT_USER***';
      }

      $where2 = "mailfix_domain_admins_users.uid ";

      if (empty($this->value)) {
        $where2 .= '<> ***CURRENT_USER***';
        $where2 = '(' . $where2 . " OR mailfix_domain_admins_users.uid IS NULL)";
      }
      else {
        $where2 .= '= ***CURRENT_USER***';
      }

      $this->query->add_where($this->options['group'], '(' . $where . ' OR ' . $where2 . ')');
    }
    else {
      // If we want reversed conditions, we want noone.
      if (empty($this->value))
        $this->query->add_where("0");
    }
  }
}
