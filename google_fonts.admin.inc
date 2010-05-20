<?php
// $Id$

/**
 * @file
 * Administrative page callbacks for the Google Fonts module.
 */

/**
 * Implementation of hook_admin_settings() for configuring the module
 */
function google_fonts_admin_settings_form(&$form_state) {

  $form = array();
  $form['introduction'] = array(
    '#value' => t('Select the fonts that you want to be available on your website. For an example of these fonts, please see the (<a href="!link">Google Fonts Directory</a>). Keep in mind that each font takes some time to download. For the best performance, only request the fonts that you are using on your website.', array('!link' => 'http://code.google.com/webfonts')),
  );
  
  $available_fonts = array();
  foreach (_google_fonts_available_fonts() as $font) {
    $available_fonts[_google_fonts_system_fontname($font)] = $font;
  }
    
  $form['google_fonts_enabled_fonts'] = array(
    '#type' => 'checkboxes',
    '#title' => t('Enabled Google Fonts'),
    '#default_value' => variable_get('google_fonts_enabled_fonts', array()),
    '#options' => $available_fonts,
    '#description' => t('After saving you can use the enabled fonts in your stylesheet.'),
  );
  
  return system_settings_form($form);
}