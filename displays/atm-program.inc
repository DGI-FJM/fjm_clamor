<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class ProgramDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $page = NULL) {
    $tabset = array(
      '#type' => 'tabset',
    );
  
    if (user_access('view fedora collection')) {
      $tabset['view-content'] = array(
        '#type' => 'tabpage',
        '#title' => t('View'),
        '#tab_name' => 'view-content',
        '#content' => theme('fjm_clamor_program', $pid, $page),
      );
    }
  
    //Allow editing of the Collection-level (really, there's just the title) Metadata
    if (user_access('edit fedora meta data')) {
      //$form = drupal_get_form('fedora_repository_edit_qdc_form', $pid, 'DC');
  
      $tabset['edit-metadata'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit Metadata'),
        '#content' => $form,
        '#tab_name' => 'edit-metadata'
      );
    }
  
    return $tabset;
  }
}
