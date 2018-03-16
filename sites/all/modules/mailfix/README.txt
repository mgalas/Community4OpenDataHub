Drupal Mailfix Module
---------------------
Author: Alex Saavedra (http://drupal.org/user/72353)
Requires: Drupal 6.x
License: GPL (see LICENSE.txt)

Description
-----------
The Mailfix module allows administrators to provide email accounts to 
Drupal users.

For this module to work you must have previously installed and configured
Postfix 2.1 or greater. A step-by-step installation guide for an Ubuntu 8.04 
server is available at: 
http://www.howtoforge.com/drupal-plus-postfix-integration-under-ubuntu-8.04

Once your Postfix server is appropriately configured you may install this
Drupal module.

Features
--------
Domain settings (admin/settings/mailfix):
* You can manage multiple domains for mail delivery
* Per domain default quota (default is 100 MB)

User settings:
* Drupal and mail accounts share the same password, thus allowing users to 
  manage their credentials from a single location.
* Users can access their mailbox using POP3, POP3S, IMAP and IMAPS
* Individual quota support for mailbox 
* Mail forwarding (very useful for disabled accounts)
* Silent BCC monitoring of specified accounts

Future features:
* Auto-responder (vacation, temporary leave)
* More translation files

Users with "administer users" privileges can configure individual settings,
such as mail quota, automatic forwarding, outgoing and incoming BCC.

Users with "administer site configuration" privileges can manage domains.

Installation
------------
See INSTALL.txt for install/upgrade instructions.

Setup
-----
- Go to "admin/settings/mailfix"
- Register all mail domains that your Drupal server manages. You must specify
  default quota for every domain.
  Once registered, all existing accounts matching your newly registered
  domain will be configured with default quota.

 
To Do
-----
- Split mailfix.module in mailfix.users.inc in order to optimize RAM
- Provide French translation
- Include "edit" operation in admin/settings/mailfix. Right now only "delete"
  operation is available.
- A sub-select sentence is used to populate mailfix_users when a new domain
  is created. Probably it is a good idea to cycle through selected users, then
  execute one INSERT each time. Are sub-selects going to work with PostgreSQL?
- include block for common tasks: 
  - list of managed mail addresses
  - list of bcc mappings
  - list of unmanaged mail addresses
- Include email and maildir fields in mailfix_users table in order to avoid
  current JOIN operators. However, in orter to make it compliant with other
  databases, the initial INSERT operation on mailfix_users upon domain creation
  should populate maildir field accordingly. Are REPLACE, SUBSTR, LOCATE, LEFT
  MySQL functions present in Postgresql and others?
  
Project Page
------------
http://drupal.org/project/mailfix