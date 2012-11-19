<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

function fjm_clamor_performer_item_autocomplete($field, $string) {
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  $map = array(
    'person' => array(
      'fq' => 'rels_hasModel_ms:"atm:personCModel"',
      'field' => 'eaccpf_complete_et',
    ),
    'instrument' => array(
      'fq' => 'rels_hasModel_ms:"atm:instrumentCModel"',
      'field' => 'dc_title_et',
    ),
    'group' => array(
      'fq' => 'rels_hasModel_ms:"atm:groupCModel"',
      'field' => 'dc_title_et',
    ),
  );
  
  $qp = new IslandoraSolrQueryProcessor();
  
  $qp->solrParams = array(
    'fq' => $map[$field]['fq'],
    'rows' => 10,
  );
  $qp->buildAndExecuteQuery("{$map[$field]['field']}:($string)");
  
  $results = array();
  foreach ((array)$qp->solrResult->response->docs as $doc) {
    $results[$doc->PID] = $doc->{$map[$field]['field']};
  }
  drupal_json($results);
  exit();
}


function fjm_clamor_performer_form(&$form_state, $pid) {
  $form = array(
    'performance' => array(
      '#type' => 'value',
      '#value' => $pid,
    ),
    'person' => array(
      '#type' => 'textfield',
      '#title' => t('Person'),
      '#description' => t('The PID of a person.  Should auto-complete on names of people.'),
      '#autocomplete_path' => 'fjm_clamor/performer/person',
    ),
    'instrument' => array(
      '#type' => 'textfield',
      '#title' => t('Instrument'),
      '#description' => t('The PID of the instrument being played.  Should auto-complete on instrument names.'),
      '#autocomplete_path' => 'fjm_clamor/performer/instrument',
    ),
    'group' => array(
      '#type' => 'textfield',
      '#title' => t('Group'),
      '#description' => t('An optional group in which the person is playing.  Should auto-complete on group names.'),
      '#autocomplete_path' => 'fjm_clamor/performer/group',
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    ),
  );
  
  return $form;
}

function fjm_clamor_performer_form_validate($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  extract($form_state['values'], EXTR_PREFIX_ALL, 'values');
  if (empty($values_person) || !valid_pid($values_person) || !fjm_clamor_model_test($values_person, 'atm:personCModel')) {
    form_set_error('person', t('Need a valid PID for "person".'));
  }
  if (empty($values_instrument) || !valid_pid($values_instrument) || !fjm_clamor_model_test($values_instrument, 'atm:instrumentCModel')) {
    form_set_error('instrument', t('Need a valid PID for "instrument".'));
  }
  if (!empty($values_group) && (!valid_pid($values_group) || !fjm_clamor_model_test($values_group, 'atm:groupCModel'))) {
    form_set_error('group', t('Group PID must be valid, if provided.'));
  }
}

function fjm_clamor_performer_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  extract($form_state['values'], EXTR_PREFIX_ALL, 'values');
  if (empty($values_group)) {
    $values_group = 'atm:unaffiliatedPerformer';
  }
  $pid = Fedora_Item::get_next_PID_in_namespace('atm');
  $item = Fedora_Item::ingest_new_item($pid, 'A', t('Performer @perf/@pid/@group/@instrument', array(
    '@perf' => $values_performance,
    '@pid' => $values_person,
    '@group' => $values_group,
    '@instrument' => $values_instrument,
  )));
  $atm_rel = 'http://digital.march.es/atmusica#';
  $item->add_relationship('hasModel', 'atm:performerCModel', FEDORA_MODEL_URI);
  $item->add_relationship('performance', $values_performance, $atm_rel);
  $item->add_relationship('player', $values_person, $atm_rel);
  $item->add_relationship('group', $values_group, $atm_rel);
  $item->add_relationship('instrument', $values_instrument, $atm_rel);
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
        'concert' => array(
          '#type' => 'item',
          '#title' => t('Concert'),
        ),
        'cycle' => array(
          '#type' => 'item',
          '#title' => t('Cycle'),
        ),
        'score' => array(
          '#type' => 'item',
          '#title' => t('Score'),
        ),
        'performers' => array(
          '#type' => 'item',
          '#title' => t('Performers'),
          'perfs' => array(
            '#prefix' => '<ul>',
            '#suffix' => '</ul>',
          ),
        ),
      ),
    );
    
    $qp = new IslandoraSolrQueryProcessor();
    $qp->solrParams = array(
      'fl' => '*,score',
    );
    $qp->buildAndExecuteQuery("PID:\"$pid\"");
    
    //XXX: Should only be one, but anyway...
    foreach ((array)$qp->solrResult->response->docs as $key => $doc) {
      $players = (array)$doc->atm_facet_player_ms;
      $pids = (array)$doc->atm_facet_player_pid_ms;
      $insts = (array)$doc->atm_facet_instrument_ms;
      $groups = (array)$doc->atm_facet_group_ms;
      $based_on = (array)$doc->rels_basedOn_ms;
      $member_of = (array)$doc->rels_isMemberOf_ms;
      $tab['performance-info']['concert']['#value'] = l($doc->atm_facet_concert_title_s, "fedora/repository/{$member_of[0]}");
      $tab['performance-info']['cycle']['#value'] = $doc->atm_facet_concert_cycle_s;
      $tab['performance-info']['score']['#value'] = l($doc->atm_performance_piece_title_s, "fedora/repository/{$based_on[0]}");
      
      foreach ($players as $i => $person) {
        $tab['performance-info']['performers']['perfs'][] = array(
          '#prefix' => '<li>',
          '#suffix' => '</li>',
          '#value' => t('!name, playing %instrument as a part of %group', array(
            '!name' => l(filter_xss($person), 'fedora/repository/' . $pids[$i]),
            '%instrument' => $insts[$i],
            '%group' => $groups[$i],
          )),
        );
      }
    }
    
    return $tab;
  }
}
