<?php

/**
 * @file
 * Default theme implementation to present profile items (values from user
 * account profile fields or modules).
 *
 * This template is used to loop through and render each field configured
 * for the user's account. It can also be the data from modules. The output is
 * grouped by categories.
 *
 * @see user-profile-category.tpl.php
 *      for the parent markup. Implemented as a definition list by default.
 * @see user-profile.tpl.php
 *      where all items and categories are collected and printed out.
 *
 * Available variables:
 * - $title: Field title for the profile item.
 * - $value: User defined value for the profile item or data from a module.
 * - $attributes: HTML attributes. Usually renders classes.
 *
 * @see template_preprocess_user_profile_item()
 */
?>
<style>
dt{
 font-weight: 200;
 margin-left: 260px;
 font-family: "Trebuchet MS", Helvetica, sans-serif;
 margin-top: -37px;
}
</style>

<dt<?php print $attributes; ?>><?php print $title; ?> <?php print $value; ?></dt>

