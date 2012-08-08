<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

function fjm_clamor_performer_form(&$form_state, $pid) {
  $form = array(
    'performance' => array(
      '#type' => 'value',
      '#value' => $pid,
    ),
    'person' => array(
      '#type' => 'textfield',
      '#title' => t('Person'),
      '#description' => t('The PID of a person.  (Will be made to autocomplete on names).'),
    ),
    'instrument' => array(
      '#type' => 'textfield',
      '#title' => t('Instrument'),
      '#description' => t('The PID of the instrument being played. (Will be made to autocomplete).'),
    ),
    'group' => array(
      '#type' => 'textfield',
      '#title' => t('Group'),
      '#description' => t('An optional group in which the person is playing. (Will be made to autocomplete).'),
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    ),
  );
  
  return $form;
}

class PerformanceDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $page = NULL) {
    $tabset = array(
      '#type' => 'tabset',
    );
    
    if (user_access('view fedora collections')) {
      $tabset += self::performanceInfoTab($pid);
    }
    
    if (user_access('edit fedora meta data')) {
      $tabset += self::addPerformerTab($pid);
    }
    
    return $tabset;
  }
  
  protected static function addPerformerTab($pid) {
    $tab = array(
      'add-performer' => array(
        '#type' => 'tabpage',
        '#title' => t('Add Performer'),
        '#tab_name' => 'add-performer',
        '#content' => drupal_get_form('fjm_clamor_performer_form', $pid),
      ),
    );
    
    return $tab;
  }
  
  protected static function performanceInfoTab($pid) {
    module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
    
    $tab = array(
      'performance-info' => array(
        '#type' => 'tabpage',
        '#title' => t('Performance Info'),
        '#tab_name' => 'performance-info',
        'performers' => array(
          '#prefix' => '<ul>',
          '#suffix' => '</ul>',
        ),
      ),
    );
    
    $qp = new IslandoraSolrQueryProcessor();
    $qp->solrParams = array(
      'fl' => '*,score',
    );
    $qp->buildAndExecuteQuery("PID:\"$pid\"");
    
    foreach ($qp->solrResult->response->docs as $doc) {
      foreach ((array)$doc->atm_facet_player_ms as $i => $person) {
        $tab['performance-info']['performers'][] = array(
          '#prefix' => '<li>',
          '#suffix' => '</li>',
          '#value' => t('!name, playing @instrument as a part of @group', array(
            '!name' => l(filter_xss($person), 'fedora/repository/' . $doc->atm_facet_player_pid_ms[$i]),
            '@instrument' => $doc->atm_facet_instrument_ms[$i],
            '@group' => $doc->atm_facet_group_ms[$i],
          )),
        );
      }
    }
    
    dsm($tab);
    return $tab;
  }
}
