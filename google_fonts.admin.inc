<?php

/**
 * @file
 * Administrative page callbacks for the Google Fonts module.
 */

/**
 * Implements hook_admin_settings() for configuring the module
 */
function google_fonts_admin_settings_form($form, &$form_state) {
  
  drupal_add_css(drupal_get_path('module', 'google_fonts') . '/google_fonts.admin.css');
  
  $form = array(
    '#attributes' => array('class' => array('google_fonts_admin_form')),
  );
  $form['introduction'] = array(
    '#markup' => t('Select the fonts that you want to be available on your website. Keep in mind that each font takes some time to download. For the best performance, only enable fonts that you are actually using on your website. More information about these fonts is available on the <a href="!link">Google font directory</a>.', array('!link' => 'http://code.google.com/webfonts')),
  );
  
  $all_fonts = _google_fonts_available_fonts();
  $form_options = array('enabled' => array(), 'disabled' => array());
  $enabled_fonts = variable_get('google_fonts_enabled_fonts', array());

  /* Load all fonts in CSS to display their example */
  $style_to_add = '';
  foreach ($all_fonts as $font) {
    $current_font_path = _google_fonts_family_pathname($font->family);
    $current_array_key = _google_fonts_family_array_key_encode($font->family);
    
    $style_to_add .= '.' . _google_fonts_family_array_key_encode($font->family) . '{ font-family: "' . $font->family . '"; } ' . PHP_EOL;
    $fonts_to_load[] = $current_font_path;
    
    if (isset($enabled_fonts[$current_array_key])) {
      $form_options['enabled'][] = $font;
    }
    else {
      $form_options['disabled'][$current_array_key] = _google_fonts_admin_font_example($font);
    }
  }
  _google_fonts_load_css($fonts_to_load);
  drupal_add_css($style_to_add, 'inline');

  /* Check if there are any enabled fonts, otherwise don't create the fieldset and display a message*/
  if (!empty($form_options['enabled'])) {
    
    $form['google_fonts_fieldset_enabled_fonts'] = array(
      '#type' => 'fieldset',
      '#title' => t('Enabled Google Fonts'),
      '#collapsible' => TRUE,
      '#collapsed' => FALSE,
      '#attributes' => array('class' => array('enabled_fonts')),
    );
    $form['google_fonts_fieldset_enabled_fonts']['enabled_fonts'] = array(
      '#type' => 'value',
      '#value' => $form_options['enabled'],
    );

    foreach ($form_options['enabled'] as $font) {  
      
      $key = _google_fonts_family_array_key_encode($font->family);
      
      if (isset($enabled_fonts[$key]['variants'])) {
        $enabled_fonts[$key]['variants'] = _google_fonts_admin_array_values($enabled_fonts[$key]['variants']);
      }
      else if (array_search('general', $font->variants)) {
        $enabled_fonts[$key]['variants'] = array('general', 'general');
      }
      else if (array_search('400', $font->variants)) {
        // a weight of 400 is usally the default when there is no general
        $enabled_fonts[$key]['variants'] = array('400', '400');
      }
      else {
        // Get the first variant (array_shift) to be selected as default
        $enabled_fonts[$key]['variants'] = array(array_shift(_google_fonts_admin_array_values($font->variants)));
      }

      $enabled_fonts[$key]['subsets'] = isset($enabled_fonts[$key]['subsets']) ? $enabled_fonts[$key]['subsets'] : 'latin';
      
      $form['google_fonts_fieldset_enabled_fonts'][$key] = array(
        
        '#type' => 'fieldset',
        '#title' => $font->family,
        '#collapsible' => FALSE,
        '#collapsed' => FALSE,
        '#description' => _google_fonts_admin_font_example($font, FALSE),
        
        $key . '_variants' => array(
          '#type' => 'checkboxes', 
          '#options' => _google_fonts_admin_array_values($font->variants),
          '#title' => t('Variants'),
          '#required' => TRUE,
          '#default_value' => $enabled_fonts[$key]['variants'],
        ),

        $key . '_subsets' => array(
          '#type' => 'radios', 
          '#options' => _google_fonts_admin_array_values($font->subsets),
          '#title' => t('Subsets'),
          '#required' => TRUE,
          '#default_value' => $enabled_fonts[$key]['subsets'],
        ),
        
        $key . '_enabler' => array(
          '#type' => 'checkbox', 
          '#title' => t('Disable this font'),
          '#default_value' => FALSE,
        ),
      );
    }
    
    // Fetch the first family name (using array_shift) to include it in the example CSS code
    $first_family = $form_options['enabled'][array_shift(array_keys($form_options['enabled']))]->family;
    $form['google_fonts_fieldset_enabled_fonts']['css_code'] = array(
      '#type' => 'textarea',
      '#title' => t('CSS code'),
      '#description' => t('The module applies CSS code for you automatically if you add it here.<br />This could be useful if you don\'t use a custom theme or if you don\'t have access to it.<br />Example: !code', array('!code' => '<code>h1#overlay-title { font-family: "' . $first_family . '"; font-size: 3em; }</code>')),
      '#resizable' => FALSE,
      '#cols' => 80,
      '#default_value' => variable_get('google_fonts_css_contents', ''),
    );
  }
  else {
    drupal_set_message(t('No Google Fonts have been enabled yet. Please enable one or more Google Fonts below.'), 'status');
  }
  
  // Check if there are any available fonts that have not been enabled yet
  // otherwise don't create the fieldset but display a message
  if (!empty($form_options['disabled'])) {
    $form['google_fonts_fieldset_disabled_fonts'] = array(
      '#type' => 'fieldset',
      '#title' => t('Available Google Fonts'),
      '#collapsible' => TRUE,
      '#collapsed' => TRUE,
      '#attributes' => array('class' => array('disabled_fonts')),
    );  
    
    $form['google_fonts_fieldset_disabled_fonts']['google_fonts_disabled_fonts'] = array(
      '#type' => 'checkboxes',
      '#default_value' => array(),
      '#options' => $form_options['disabled'],
    );
  }
  else {
    drupal_set_message(t('All available Google Fonts have been enabled.'), 'status');
  }
  
  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => t('Save configuration'),
    '#weight' => 5,
  );
  
  return $form;
}

/**
 * Submit handler of the admin settings form
 * Saves the fonts, CSS code and settings
 */
function google_fonts_admin_settings_form_submit($form, &$form_state) {
  $enabled_fonts = array();

  if (isset($form_state['values']['enabled_fonts'])) {
    foreach ($form_state['values']['enabled_fonts'] as $key => $font) {
      $font_key = _google_fonts_family_array_key_encode($font->family);
      
      // Strip out unchecked font variants
      foreach ($form_state['values'][$font_key . '_variants'] as $variant_key => $variant) {
        if (!$variant) {
          unset ($form_state['values'][$font_key . '_variants'][$variant_key]);
        }
      }
      
      // Only add the font if the enabler checkbox is not checked
      if (!$form_state['values'][$font_key . '_enabler']) {
        $enabled_fonts[$font_key] = array(
          'family' => $font->family,
          'variants' => $form_state['values'][$font_key . '_variants'],
          'subsets' => $form_state['values'][$font_key . '_subsets'],
        );
      }
    }
  }
  
  foreach ($form_state['values']['google_fonts_disabled_fonts'] as $key => $font) {
    if (!empty($font)) {
      $enabled_fonts[$key] = array('family' => $font);
    }
  }

  // Save the font settings to the database
  variable_set('google_fonts_enabled_fonts', $enabled_fonts);
  
  // Save the CSS code. If left empty, just delete the existing variable
  if (empty($form_state['values']['css_code'])) {
    variable_del('google_fonts_css_contents');
  }
  else {
    variable_set('google_fonts_css_contents', $form_state['values']['css_code']);
    
    // Save the CSS code as local file
    if (!_google_fonts_cache($form_state['values']['css_code'], TRUE)) {
      drupal_set_message(t('Could not generate the CSS code for the Google Fonts.'), 'error');
    }
  }
  
  drupal_set_message(t('Configuration saved'), 'status');
}

/** 
 * Returns the font family name, including a span with added styling
 */
function _google_fonts_admin_font_example($font, $display_with_icons = TRUE) {
  $class_name = _google_fonts_family_array_key_encode($font->family);
  $example = '<span class="' . $class_name . ' font_example">' . $font->family . '</span>';
  
  if ($display_with_icons) {
    if (sizeof($font->variants) > 1) {
      $example .= '<span class="variants">' . t('<span>@count</span> variants', array('@count' => sizeof($font->variants))) . '</span>';
    }

    if (sizeof($font->subsets) > 1) {
      $example .= '<span class="subsets">' . t('<span>@count</span> subsets (@subsets)', array('@count' => sizeof($font->subsets), '@subsets' => implode(', ', $font->subsets))) . '</span>';
    }
  }
  
  return $example;
}

/** 
 * Return an array with the same keys as values
 */
function _google_fonts_admin_array_values($array_in) {
  $array_out = array();
  foreach ($array_in as $value) {
    $array_out[$value] = $value;
  }
  return $array_out;
}
