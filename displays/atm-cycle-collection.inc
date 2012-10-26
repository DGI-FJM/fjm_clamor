<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class CycleCollectionDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $page_number = NULL)
  {
    $tabset = array();
    
    if (user_access('view fedora collection')) {
      $collection_query = <<<END_QUERY
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX atm-rel: <http://digital.march.es/atmusica#>
PREFIX dc: <http://purl.org/dc/elements/1.1/>
SELECT ?object ?title ?thumbnail
FROM <#ri>
WHERE {
  ?object fm:hasModel <info:fedora/atm:concertCycleCModel> ;
          fm:state fm:Active ;
          dc:title ?title .
  OPTIONAL {
    ?thumbnail atm-rel:isIconOf ?object ;
               fm:state fm:Active .
  } .
}
ORDER BY ?title
END_QUERY;
      module_load_include('inc', 'fedora_repository', 'CollectionClass');
      module_load_include('inc', 'fedora_repository', 'ObjectHelper');
      $results = ObjectHelper::performSparqlQuery($collection_query);
      $tabset['view'] = array(
        '#type' => 'tabpage',
        '#title' => t('View'),
        '#tab_name' => 'view-collection',
        'content' => CollectionClass::assembleCollectionView($results),
      );
    }
    
    if (user_access('ingest new fedora objects')) {
      $tabset['add-child'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add Cycle'),
        '#tab_name' => 'add-child',
        '#content' => drupal_get_form('fjm_clamor_cycle_create_form', $pid),
      );
    }
    
    return $tabset;
  }
}

function fjm_clamor_cycle_create_form(&$form_state, $pid) {
  return array(
    'title' => array(
      '#type' => 'textfield',
      '#title' => t('Cycle Name'),
      '#required' => TRUE,
    ),
    'parent' => array(
      '#type' => 'value',
      '#value' => $pid,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Create Cycle'),
    ),
  );
}

function fjm_clamor_cycle_create_form_submit(&$form, &$form_state) {
  $name = $form_state['values']['title'];
  $parent = $form_state['values']['parent'];
  
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  $pid = Fedora_Item::get_next_PID_in_namespace(variable_get('fjm_clamor_namespace', 'atm'));
  $message = '';
  $item = Fedora_Item::ingest_new_item($pid, 'A', $name);
  
  if ($item) {
    $message = t('Created item !item.', array(
      '!item' => l($item->pid, "fedora/repository/{$item->pid}")
    ));
    
    if ($item->add_relationship('isMemberOfCollection', $parent)) {
      $message .= ' ' . t('Added collection membership relation to !item.', array(
        '!item' => l($item->pid, "fedora/repository/{$item->pid}")
      ));
    }
    else {
      $message .= ' ' . t('Failed adding collection membership relation to !item.', array(
        '!item' => l($item->pid, "fedora/repository/{$item->pid}")
      ));
    }
    
    if ($item->add_relationship('hasModel', 'atm:concertCycleCModel', FEDORA_MODEL_URI)) {
      $message .= ' ' . t('Added model relationship to !item.', array(
        '!item' => l($item->pid, "fedora/repository/{$item->pid}")
      ));
    }
    else {
      $message .= ' ' . t('Failed adding model relationship to !item.', array(
        '!item' => l($item->pid, "fedora/repository/{$item->pid}")
      ));
    }
  }
  
  $form_state['redirect'] = array("fedora/repository/{$item->pid}");
  drupal_set_message($message, 'info');
  
}
