<?php
/**
 * @file
 * Generic functions shared by Mailfix functionality.
 */

/**
 * Helper function verifying if an email address belongs to registered domains.
 *
 * When user profile is being updated, we must verify if provided mail address
 * belongs to registered domains.
 *
 * @param $email
 *    Email address who's domain will be checked.
 * @return 
 *    0 if email's domain is not a registered one
 *    positive integer otherwise, corresponding to domain_id
 */
function mailfix_domain_id_from_email($email) {
  $domain = drupal_substr(strrchr($email, "@"), 1); // extract domain
  $domain_id = 0;
  $query = db_select('mailfix_domains', 'md');
  $query->condition('md.domain_name', $domain, "=")
    ->fields('md', array('domain_id'));
  $result = $query->execute()
    ->fetchField();
  return $result;
} // mailfix_domain_id_from_email

