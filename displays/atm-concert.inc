<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

function theme_fjm_clamor_draggable_table($elements) {
  $header = array();
  $rows = array();
  foreach (array_values(element_children($elements)) as $index => $key) {
    //Each child becomes a row...
    $element = $elements[$key];
    $row = array(
      'class' => 'draggable',
      'data' => array(),
    );
    
    foreach (element_children($element) as $idx => $t) {
      $e = $element[$t];
      if ($index === 0) {
        $header[$idx] = array(
          'data' => $e['#title'],
        );
      }
      unset($e['#title']);
      $row['data'][] = array(
        'data' => drupal_render($e),
      );
    }
    
    $rows[] = $row;
  }

  $markup = theme('table', $header, $rows, array('id' => 'movement-table'));
  drupal_add_tabledrag('movement-table', 'order', 'group', 'movement-row-weight');
  return $markup;
}

/**
 * Get an array of all the files in this directory (including subdirectories
 * if $recurse is TRUE).
 * XXX: This could be made a great deal more efficient in the case of a deep hierarchy, but anyway.
 */
function _fjm_clamor_traverse_directory($filesystem_base, $string, $sub_dir = '.', $recurse = TRUE) {
  $files = array();

  $dir_name = "$filesystem_base/$sub_dir";

  if ($sub_dir === '.') {
    $dir_name = $filesystem_base;
  }

  if (is_dir($dir_name)) {
    $dir = dir($dir_name);
    while ($entry = $dir->read()) {
      if (in_array($entry, array('.', '..'))) {
        continue;
      }
      else {
        $this_entry = "$sub_dir/$entry";
        if ($sub_dir === '.') {
          $this_entry = $entry;
        }

        if ($recurse && is_dir("$filesystem_base/$this_entry")) {
          $files += _fjm_clamor_traverse_directory($filesystem_base, $string, $this_entry, $recurse);
        }
        elseif ($string === '*' || strpos($entry, $string) === 0) { //XXX: Should probably get rid of the wildcard.
          $files["$filesystem_base/$this_entry"] = $entry;
        }
      }
    }
    $dir->close();
  }

  return $files;
}

function fjm_clamor_movement() {
  $args = func_get_args();
  if (empty($args)) {  //Being used as AHAH callback.
    module_load_include('inc', 'php_lib', 'Ahah');
    list($form_id, $form_build_id, $form, $form_state) = Ahah::getFormInfo();
    $processed_form = Ahah::processForm($form_id, $form, $form_state);
    $parents = $form_state['clicked_button']['#array_parents'];
    $button = array_pop($parents);
    if ($button == 'add_movement') {
      //If adding, add a movement to the end of the list.
      $form_state['storage']['movements'][] = array(
        'name' => $form_state['values']['new_movement']['name'],
        'identifier' => $form_state['values']['new_movement']['file'],
        'weight' => count($form_state['storage']['movements']),
      );
    }
    elseif ($button == 'delete_movement') {
      //If removing, remove the proper movement.
      $current =& $form_state['values']['movements'][array_shift($parents)];
      while ($key = array_shift($parents)) {
        $current =& $current[$key];
      }
      $current = NULL;
      $form_state['storage']['movements'] = array_filter($form_state['values']['movements']);
    }
    
    //Rebuild.
    $form = Ahah::rebuildForm($form_id, $form_build_id, $form, $form_state);
    
    //Respond.
    Ahah::respond($form['movements']);
  }
  else { //Being used for autocomplete.
    $dir = '/mnt/islandora/ToIngest/clamor/Audio_conciertos';
    $subdir = '.';
    $string = array_pop($args);
    if (count($args) > 0) {
      $subdir = implode('/', $args);
    }
    
    drupal_json(_fjm_clamor_traverse_directory($dir, $string, $subdir));
    exit();
  }
}

function fjm_clamor_score_autocomplete($string = '') {
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  //Perform Solr search on both score titles and composer names (make edge'd fields!)
  $qp = new IslandoraSolrQueryProcessor();
  
  $qp->solrParams += array(
    'qf' => 'PID^16.0 atm_score_title_et^4.0 atm_score_composer_et',
    'fq' => array(
      'rels_hasModel_ms:"atm:scoreCModel"',
    ),
    'rows' => 10,
    'fl' => 'PID,atm_score_title_es,atm_score_composer_es',
  );
  
  $qp->buildAndExecuteQuery($string,
    array(
      'type' => 'dismax',
    )
  );
  
  $results = array();
  foreach ((array)$qp->solrResult->response->docs as $doc) {
    $results[$doc->PID] = t('@pid: %title by @composer', array(
      '@pid' => $doc->PID,
      '%title' => $doc->atm_score_title_es,
      '@composer' => $doc->atm_score_composer_es,
    ));
  }
  drupal_json($results);
  exit();
}

function fjm_clamor_performance_ingest_form(&$form_state, $pid = NULL) {
  module_load_include('inc', 'php_lib', 'Ahah');
  Ahah::get_ahah_js();
  
  if (empty($form_state['storage'])) {
    $form_state['storage'] = array();
  }
  $storage =& $form_state['storage'];

  $hier_ops = array(
    'performance' => t('Single file for performance'),
    'movements' => t('Audio files broken down into movements'),
    'none' => t('No MP3/Audio'),
  );
  $form = array();
  if ($storage['page'] === NULL) {
    //Select a score (from Fedora), and whether or not this performance will
    //  contain an MP3 directly, contain MP3s in movmements, or not contain
    //  an MP3 at all.
    $form = array(
      'score' => array(
        '#type' => 'textfield',
        '#title' => t('Score'),
        '#description' => t('The score on which this performance is based. (Make autocomplete on title or composer)'),
        '#required' => TRUE,
        '#default_values' => $form_state['values']['score'],
        '#autocomplete_path' => 'fjm_clamor/score_autocomplete',
      ),
      'hierarchy' => array(
        '#type' => 'select',
        '#title' => t('Object Hierarchy'),
        '#description' => t('How audio files will be related to this item.'),
        '#options' => $hier_ops,
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Next Page'),
      ),
    );
    $storage['parent_pid'] = $pid;
  }
  elseif ($storage['page'] === 1) {
    $form = array(
      '#tree' => TRUE,
      'score' => array(
        '#type' => 'item',
        '#title' => t('Score'),
        '#description' => t('The score on which this performance is based.'),
        '#value' => (isset($storage['score'])? $storage['score']:$form_state['values']['score']),
      ),
      'hierarchy' => array(
        '#type' => 'item',
        '#title' => t('Object Hierarchy'),
        '#description' => t('How audio files will be related to this item.'),
        '#value' => $hier_ops[$storage['hierarchy']],
      ),
    );
   
    //If contains MP3; show selection (autocomplete or file upload?)
    if ($storage['hierarchy'] == 'performance') {
      $form['mp3'] = array(
        '#tree' => FALSE,
        '#type' => 'upload_element',
        '#title' => t('Audio file'),
        '#description' => t('Select the audio file representing this performance.'),
      );
    }
    //Else if has MP3s in movements, setup AHAH stuff for adding multiple (orderable?) items.
    elseif ($storage['hierarchy'] == 'movements') {
      $form['movements'] = array(
        '#theme' => 'fjm_clamor_draggable_table',
        '#prefix' => '<div id="movement-table-wrapper">',
        '#suffix' => '</div>',
      );

      
      if (empty($form_state['values']['movements'])) {
        $form_state['values']['movements'][] = array(
          'name' => '',
          'identifier' => '',
          'weight' => 1,
        );
      }
      
      if (!isset($form_state['storage']['movements'])) {
        $form_state['storage']['movements'] = array();
      }
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
            '#value' => $movement['name'],
          ),
          'identifier' => array(
            '#type' => 'item',
            '#title' => t('MP3?'),
            '#value' => isset($movement['identifier'])?t('Has file'):t('No file'),
          ),
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
    }

    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Ingest'),
    );
  }
  $form['#action'] = "#performance-ingest";
  return $form;
}

function fjm_clamor_performance_ingest_form_validate($form, $form_state) {
  $storage = $form_state['storage'];

  if ($storage['page'] === NULL) {
    //Check that selected score is really a score object.
    //Check that the selected hierarchy is valid.
  }
  elseif ($storage['page'] === 1) {
    
  }
}

function fjm_clamor_performance_ingest_form_submit(&$form, &$form_state) {
  $storage =& $form_state['storage'];
  $values =& $form_state['values'];
  
  if ($storage['page'] === NULL) {
    //Store path to the selected score, and whether how/if we are adding MP3s.
    $storage['score'] = $values['score'];
    $storage['hierarchy'] = $values['hierarchy'];

    $storage['page'] = 1;
    if ($storage['hierarchy'] == 'none') {
      fjm_clamor_performance_ingest_form_submit($form, $form_state);
    }
  }
  elseif ($storage['page'] === 1) {
    //Get'er done!
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
      file_delete($form_state['storage']['movements'][$current['key']]->filepath);
      unset($form_state['storage']['movements'][$current['key']]);
      $form_state['rebuild'] = TRUE;
    }

    // Change order of the movements in storage, based on the order of elements
    //  of the rows.
    $movements = array();
    if (isset($form_state['clicked_button']['#post']['movements'])) {
      foreach ($form_state['clicked_button']['#post']['movements'] as $key => $mov) {
        if (isset($form_state['storage']['movements'][$key])) {
          $movements[] = $form_state['storage']['movements'][$key];
        }
      }
    }
    $form_state['storage']['movements'] = $movements;

    if ($button == 'add_movement') {
      //If adding, add a movement to the end of the list.
      $form_state['storage']['movements'][] = array(
        'name' => $form_state['values']['new_movement']['name'],
        'identifier' => $form_state['values']['file'],
      );
      $form_state['rebuild'] = TRUE;
    }
    elseif ($button == 'submit') {
      module_load_include('inc', 'fedora_item', 'api/fedora_item');
      // Create the performance object
      $pid = Fedora_Item::get_next_PID_in_namespace('atm');
      $item = Fedora_Item::ingest_new_item($pid, 'A', 'Performance (get name from related score)');
      $item->add_relationship('hasModel', 'atm:performanceCModel', FEDORA_MODEL_URI);
      //Add the relation to the concert
      $item->add_relationship('isMemberOf', $form_state['storage']['parent_pid']);
      // Relate the performance to its score.
      $item->add_relationship('basedOn', $storage['score'], 'http://digital.march.es/atmusica#');
      // Add the order literal.
      $order = 0;
      $item->add_relationship('concertOrder', $order, 'http://digital.march.es/atmusica#', RELS_TYPE_PLAIN_LITERAL);
      // If hierarchy is performance
      if ($storage['hierarchy'] == 'performance') {
      //   add MP3 if provided.
        if (is_readable($values['mp3']->filepath)) {
          fjm_clamor_fix_mp3($values['mp3']->filepath);
          $item->add_datastream_from_file($values['mp3']->filepath, 'MP3', 'MP3 datastream', $values['mp3']->filemime);
          file_delete($values['mp3']->filepath);
        }
        else {
          drupal_set_message(t('Could not read file "@file" for @pid.', array(
            '@file' => $movement['identifier'],
            '@pid' => $item->pid,
          )), 'warning');
        }
      }
      // If hierarchy is movements...
      elseif ($storage['hierarchy'] == 'movements') {
      //   For each movement
        $pids = Fedora_Item::get_next_PID_in_namespace('atm', count($values['movements']));
        foreach (array_values($storage['movements']) as $key => $movement) {
      //     Create a movement object.
          $m_item = Fedora_Item::ingest_new_item(array_shift($pids), 'A', $movement['name']);
          $m_item->add_relationship('hasModel', 'atm:movementCModel', FEDORA_MODEL_URI);
      //     Relate it to the performance.
          $m_item->add_relationship('isMemberOf', $item->pid, RELS_EXT_URI);
      //     Add the order literal.
          $m_item->add_relationship('pieceOrder', $key, 'http://digital.march.es/atmusica#', RELS_TYPE_PLAIN_LITERAL);
      //     Add the MP3
          if (is_readable($movement['identifier']->filepath)) {
            fjm_clamor_fix_mp3($movement['identifier']->filepath);
            $m_item->add_datastream_from_file($movement['identifier']->filepath, 'MP3', 'MP3 datastream', $movement['identifier']->filemime);
            file_delete($movement['identifier']->filepath);
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
      unset($form_state['storage']);
    }
  }
}

function fjm_clamor_lecture_ingest_form(&$form_state, $pid = NULL) {
  $form = array(
    '#tree' => FALSE,
    'parent_pid' => array(
      '#type' => 'value',
      '#value' => $pid,
    ),
    'file' => array(
      '#tree' => FALSE,
      '#type' => 'upload_element',
      '#required' => TRUE,
      '#title' => t('Audio file'),
      '#file_validators' => array(
        'fjm_clamor_file_is_mime' => array('audio/mpeg'),
      ),
    ),
    'description' => array(
      '#type' => 'textarea',
      '#title' => t('Description'),
      '#description' => t('A short description of the lecture.'),
      '#required' => TRUE,
    ),
    'submit' => array(
      '#type' => 'submit',
      '#value' => t('Submit'),
    ),
    '#attributes' => array(
      'enctype' => 'multipart/form-data',
    ),
  );
  
  return $form;
}

function fjm_clamor_lecture_ingest_form_validate(&$form, &$form_state) {
}

function fjm_clamor_lecture_ingest_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  
  $new_pid = Fedora_Item::get_next_PID_in_namespace('atm');
  $parent_pid = $form_state['values']['parent_pid'];
  $item = Fedora_Item::ingest_new_item($new_pid, 'A', 'Lecture in ' . $parent_pid);
  $item->add_relationship('hasModel', 'atm:lectureCModel', FEDORA_MODEL_URI);
  $item->add_relationship('isMemberOf', $parent_pid);
  fjm_clamor_fix_mp3($form_state['values']['file']->filepath);
  $item->add_datastream_from_file($form_state['values']['file']->filepath,
    'MP3',
    'MP3 datastream',
    $form_state['values']['file']->filemime
  );
  
  if (!empty($form_state['values']['description'])) {
    fjm_clamor_set_dc_description($new_pid, $form_state['values']['description']);
  }
}

class ConcertDisplay extends BasicDisplay {
  public static function addTabsStatically($pid, $piece_index = NULL) {
    $tabset = array();
    $i = 0;
    if (user_access('view fedora collection')) {
      $tabset['view'] = array(
          '#type' => 'tabpage',
          '#title' => t('View'),
          '#content' => theme('fjm_clamor_concert', $pid, $piece_index),
          '#tab_name' => 'view-content',
          '#weight' => $i++,
      );
    }
    
    //Allow addition of Performances to the Concert
    if (user_access('ingest new fedora objects')) {
      $tabset['performance-ingest'] = array(
          '#type' => 'tabpage',
          '#title' => t('Add New Performance'),
          '#weight' => $i++,
          '#tab_name' => 'performance-ingest',
          '#content' => drupal_get_form('fjm_clamor_performance_ingest_form', $pid),
      );
      $tabset['lecture-ingest'] = array(
          '#type' => 'tabpage',
          '#title' => t('Add New Lecture'),
          '#weight' => $i++,
          '#tab_name' => 'lecture-ingest',
          '#content' => drupal_get_form('fjm_clamor_lecture_ingest_form', $pid),
      );
      $tabset['program-info'] = self::get_program_tab($pid);
      $tabset['program-info']['#weight'] = $i++;
      $tabset += self::getImageTab($pid, $i++);
    }
    
    return $tabset;
  }
  
  private static function get_program_tab($pid) {
    $program_exists = self::get_program($pid);
    return array(
      '#type' => 'tabpage',
      '#title' => ($program_exists?
        t('Program Info'):
        t('Add Program')),

      '#tab_name' => 'program-info',
      '#content' => ($program_exists?
        FALSE:
        drupal_get_form('fedora_repository_ingest_form', $pid, self::get_title($pid), 'atm:programCModel')),
    );
  }
  
  private static function get_program($pid) {
      module_load_include('inc', 'fedora_repository', 'ObjectHelper');
      $results = ObjectHelper::performSparqlQuery(<<<EOQ
SELECT ?obj {
  ?obj <fedora-rels-ext:isMemberOf> <info:fedora/$pid> ;
       <fedora-model:state> <fedora-model:Active> ;
       <fedora-model:hasModel> <info:fedora/atm:programCModel> .
}
EOQ
      );

      return $results[0]['obj'];
  }
}
