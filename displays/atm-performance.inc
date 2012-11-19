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
      if (!self::hasMP3($pid)) {
        $tabset += self::modifyMovementTab($pid);
      }
      $tabset += self::addPerformerTab($pid);
    }
    
    return $tabset;
  }

  protected static function modifyMovementTab($pid) {
    return array(
      'modify-movements' => array(
        '#type' => 'tabpage',
        '#title' => t('Modify Movements'),
        '#tab_name' => 'modify-movements',
        '#content' => drupal_get_form('fjm_clamor_modify_movements_form', $pid),
      ),
    );
  }

  protected static function hasMP3($pid) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);

    return in_array('MP3', $item->datastreams);
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

function fjm_clamor_modify_movements_form(&$form_state, $pid) {
  // Load the movement info into storage.
  if (empty($form_state['storage']['movements'])) {
    $form_state['storage']['movements'] = array();

    module_load_include('inc', 'fedora_repository', 'ObjectHelper');
    $results = ObjectHelper::performSparqlQuery(<<<EOQ
PREFIX atm-rel: <http://digital.march.es/atmusica#>
SELECT ?name ?identifier ?key
FROM <#ri>
WHERE {
  ?identifier <fedora-rels-ext:isMemberOf> <info:fedora/$pid> ;
              atm-rel:pieceOrder ?key ;
              <fedora-model:label> ?name .
}
EOQ
);

    foreach ($results as $movement) {
      $form_state['storage']['movements'][$movement['key']] = $movement;
    }

    ksort($form_state['storage']['movements']);
  }
  


  $form = array(
    '#tree' => TRUE,
  );
  $form['performance_pid'] = array(
    '#type' => 'value',
    '#value' => $pid,
  );
  $form['movements'] = array(
    '#theme' => 'fjm_clamor_draggable_table',
    '#prefix' => '<div id="movement-table-wrapper">',
    '#suffix' => '</div>',
  );

  foreach ($form_state['storage']['movements'] as $key => $movement) {
    // Should be a pid while editing...  Is this looking a little too far forwards?
    $form['movements'][$key] = array(
      'key' => array(
        '#type' => 'value',
        '#value' => $key,
      ),
      'name' => array(
        '#type' => 'item',
        '#title' => t('Movement Name'),
        '#value' => l($movement['name'], "fedora/repository/{$movement['identifier']}"),
      ),
      //'identifier' => array(
      //  '#type' => 'item',
      //  '#title' => t('MP3?'),
      //  '#value' => isset($movement['identifier'])?t('Has file'):t('No file'),
      //),
      'delete_movement' => array(
        '#type' => 'submit',
        '#title' => t('Delete'),
        '#name' => 'delete--' . $key,
        '#value' => t('Delete movement association'),
      ),
      'weight' => array(
        '#type' => 'weight',
        //'#title' => t('Weight'),
        '#default_value' => $i++,
        '#attributes' => array(
         'class' => 'movement-row-weight',
        ),
      ),
    );
  }

  $form['rearrange'] = array(
    '#type' => 'submit',
    '#value' => t('Commit movement order'),
  );

  $form['new_movement'] = array(
    '#type' => 'fieldset',
    '#title' => t('New Movement'),
    'name' => array(
      '#type' => 'textfield',
      '#title' => t('Movement name'),
      '#description' => t('The name of the movement.'),
    ),
    'file' => array(
      '#type' => 'upload_element',
      '#title' => t('MP3 file'),
      '#tree' => FALSE,
    ),
    'add_movement' => array(
      '#type' => 'submit',
      '#value' => t('Add movement'),
    ),
  );

  $form['#action'] = "#modify-movements";

  return $form;
}

function fjm_clamor_modify_movements_form_validate($form, $form_state) {
  // Nothing to do?
}

function fjm_clamor_modify_movements_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');

  $parents = $form_state['clicked_button']['#array_parents'];
  $button = array_pop($parents);

  if ($button == 'delete_movement') {
    //If removing, remove the proper movement.
    // (Get the "key" value, and unset the entry in the $form_state['storage']['movements'] array
    $current =& $form_state['values'][array_shift($parents)];
    $key = array_shift($parents);
    while ($key !== NULL) {
      $current =& $current[$key];
      $key = array_shift($parents);
    }

    $m_item = new Fedora_Item($form_state['storage']['movements'][$current['key']]['identifier']);
    $m_item->purge_relationships('isMemberOf', "info:fedora/{$form_state['values']['performance_pid']}", RELS_EXT_URI);
    unset($m_item);
    unset($form_state['storage']['movements'][$current['key']]);
  }

  // Change order of the movements in storage, based on the order of elements
  //  of the rows. ('#post' has them in the order in which they were posted)
  $movements = array();
  if (isset($form_state['clicked_button']['#post']['movements'])) {
    foreach ($form_state['clicked_button']['#post']['movements'] as $key => $mov) {
      if (isset($form_state['storage']['movements'][$key])) {
        $current_key = count($movements);
        if ($key != $current_key) {
          // The position of this one changed, so update its RELS-EXT....

          // ... Get the object.
          $m_item = new Fedora_Item($form_state['storage']['movements'][$key]['identifier']);
          //     Purge the old relationship.
          $m_item->purge_relationships('pieceOrder', NULL, 'http://digital.march.es/atmusica#');
          //     Add the new order literal.
          $m_item->add_relationship('pieceOrder', $current_key, 'http://digital.march.es/atmusica#', RELS_TYPE_PLAIN_LITERAL);
          unset($m_item);
        }
        $movements[] = $form_state['storage']['movements'][$key];
      }
    }
  }
  $form_state['storage']['movements'] = $movements;

  if ($button == 'add_movement') {
    $pid = Fedora_Item::get_next_PID_in_namespace('atm');
    $key = count($form_state['storage']['movements']);
    //     Create a movement object.
    $m_item = Fedora_Item::ingest_new_item($pid, 'A', $form_state['values']['new_movement']['name']);
    $m_item->add_relationship('hasModel', 'atm:movementCModel', FEDORA_MODEL_URI);
    //     Relate it to the performance.
    $m_item->add_relationship('isMemberOf', $form_state['values']['performance_pid'], RELS_EXT_URI);
    //     Add the order literal.
    $m_item->add_relationship('pieceOrder', $key, 'http://digital.march.es/atmusica#', RELS_TYPE_PLAIN_LITERAL);

    $form_state['storage']['movements'][] = array(
      'identifier' => $pid,
      'name' => $form_state['values']['new_movement']['name'],
      'key' => $key,
    );

    //     Add the MP3
    if (is_readable($form_state['values']['file']->filepath)) {
      fjm_clamor_fix_mp3($form_state['values']['file']->filepath);
      $m_item->add_datastream_from_file($form_state['values']['file']->filepath, 'MP3', 'MP3 datastream', $form_state['values']['file']->filemime);
      file_delete($form_state['values']['file']->filepath);
    }
    else {
      drupal_set_message(t('Could not read file "@file" for @pid.', array(
        '@file' => $movement['identifier'],
        '@pid' => $m_item->pid,
      )), 'warning');
    }
    $m_item->forget();
  }
}
