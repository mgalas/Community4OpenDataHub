<?php

/**
 * @file
 * Provides editor view node access filter
 */
 
class mailfix_views_handler_field_mailfix_domain_admin_access extends views_handler_field_boolean {
  function render($values) {
    global $user;

    if ($user->uid != 1)
      $value = empty($values->mailfix_domain_admins_uid) ? 0 : 1;
    else
      $value = 1;

    if (!empty($this->options['not'])) {
      $value = !$value;
    }

    $values->mailfix_domain_admins_uid = $value;
    return parent::render($values);
  } 

  function query() {
    global $user;

    if ($user->uid != 1 && !user_access('administer users')) {
      $table = $this->ensure_my_table();
      $join = new views_join();
      $join->construct('mailfix_users', $this->table_alias, 'uid', 'uid');
      $this->query->ensure_table('mailfix_users', $this->relationship, $join);

      $this->query->distinct = TRUE;
      // We use an IF for MySQL/PostgreSQL compatibility. Otherwise PostgreSQL
      // would return 'f' and 't'.
      $sql = "IF(mailfix_domain_admins.uid = ***CURRENT_USER***, 1, 0)";
      // We liberally steal from views_handler_sort_formula and
      // views_handler_filter_search here.
      $this->field_alias = $this->query->add_field(NULL, $sql, $this->table_alias . '_' . $this->field, array('aggregate' => TRUE));
    }
    else
      $this->field_alias = $this->query->add_field(NULL, "1", $this->table_alias . '_' . $this->field);
  }
}
