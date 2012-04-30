<?php

module_load_include('inc', 'fjm_clamor/basic-display');

function fjm_clamor_performance_form(&$form_state, $pid = NULL) {
  $storage =& $form_state['storage'];
  
  if ($storage['page'] === NULL) {
    //Select a score (from Fedora), and whether or not this performance will
    //  contain an MP3 directly, contain MP3s in movmements, or not contain
    //  an MP3 at all.
    return array(
      'score' => array(
        '#type' => 'textfield',
        '#title' => t('Score'),
        '#description' => t('The score on which this performance is based. (Make autocomplete on title or composer)'),
      ),
      'hierarchy' => array(
        '#type' => 'select',
        '#title' => t('Object Hierarchy'),
        '#description' => t('How audio files will be related to this item.'),
        '#options' => array(
          'performance' => t('Single file for performance'),
          'movements' => t('Audio files broken down into movements'),
          'none' => t('No MP3/Audio'),
        ),
      ),
      'submit' => array(
        '#type' => 'submit',
        '#value' => t('Next Page'),
      ),
    );
  }
  elseif ($storage['page'] === 1) {
    $form = array(
      'score' => array(
        '#type' => 'item',
        '#title' => t('Score'),
        '#description' => t('The score on which this performance is based. (Make autocomplete on title or composer)'),
        '#value' => $storage['score'],
      ),
      'hierarchy' => array(
        '#type' => 'item',
        '#title' => t('Object Hierarchy'),
        '#description' => t('How audio files will be related to this item.'),
        '#value' => $storage['hierarchy'],
      ),
    );
    
    //If contains MP3; show selection (autocomplete or file upload?)
    if ($storage['hierarchy'] == 'performance') {
      $form['mp3'] => array(
        '#type' => 'textfield',
        '#title' => t('Audio file'),
        '#description' => t('Select the audio file representing this performance.'),
        //'#autocomplete_path' => 'fjm_clamor/performance/autocomplete'
      );
    }
    //Else if has MP3s in movements, setup AHAH stuff for adding multiple (orderable?) items.
    elseif ($storage['hierarchy'] == 'movements') {
      $form['movements'] = array();
      
      if (!$storage['movements']) {
        $storage['movements'][] = array(
          'path' => '',
          'weight' => 0,
        );
      }
      
      foreach ($storage['movements'] as $key => $movement) {
        $is_pid = $movement['pid']; // Should be a pid while editing...  Is this looking a little too far forwards?
        
        $form['movements'][$key] = array(
          'weight' => array(
            '#type' => 'weight',
            //'#title' => t('Weight'),
            '#default_value' => $movement['weight'],
          ),
          'identifier' => array(
            '#type' => 'textfield',
            '#title' => t('Movement path/ID'),
            '#default_value' => ($is_pid? $movement['pid']: $movement['path']),
          ),
          'delete_movement--' . $key => array(
            '#type' => 'submit',
            '#value' => t('Delete movement association'),
          ),
        );
      }
      
      $form['add_movement'] = array(
        '#type' => 'submit',
        '#value' => t('Add movement'),
      );
    }
    //Otherwise, shouldn't be here...
    else {
      form_set_error('hierarchy', t('Bad hierarchy!?'));
      return $form;
    }
    
    $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Ingest'),
    );
  }
}

function fjm_clamor_performance_form_validate($form, $form_state) {
  $storage = $form_state['storage'];

  if ($storage['page'] === NULL) {
    //Check that selected score is really a score object.
    //Check that the selected hierarchy is valid.
  }
  elseif ($storage['page'] === 1) {
    
  }
}

function fjm_clamor_performance_form_submit($form, &$form_state) {
  $storage =& $form_state['storage'];
  $values =& $form_state['values'];
  
  if ($storage['page'] === NULL) {
    //Store path to the selected score, and whether how/if we are adding MP3s.
    $storage['score'] = $values['score'];
    $storage['hierarchy'] = $values['hierarchy'];
    
    if ($storage['hierarchy'] != 'none') {
      $form_state['rebuild'] = TRUE;
      $storage['page'] = 1;
    }
  }
  elseif ($storage['page'] === 1) {
    if (strpos($values['op'], 'add_movement') === 0) {
      $storage['movements'][] = array(
        'weight' => 100,
        'path' => '',
      );
    }
    elseif (strpos($values['op'], 'delete_movement--') === 0) {
      $index = int_val(substr($values['op'], strrpos($values['op'], '-')));
      if ($storage['movements'][$index]) {
        unset($storage['movements'][$index]);
      }
      else {
        form_set_error('', t('Tried to remove a non-existent movement!?'));
      }
      $form_state['rebuild'] = TRUE;
    }
    elseif (strpos($values['op'], 'submit') === 0) {
      //Get'er done!
    }
  }
}

class ConcertDisplay extends BasicDisplay {
  public function addTabs($piece_index = NULL) {
    $tabset = array();
    
    if (user_access('view fedora collection')) {
      $tabset['view'] = array(
        '#type' => 'tabpage',
        '#title' => t('View'),
        '#content' => theme('fjm_clamor_concert', $this->pid, $piece_index),
        '#tab_name' => 'view-content',
      );
    }
    
    //Allow editing of the Concert Metadata
    if (user_access('edit fedora meta data')) {
      $form = drupal_get_form('fedora_repository_edit_qdc_form', $this->pid, 'MODS');
    
      $tabset['edit_metadata'] = array(
        '#type' => 'tabpage',
        '#title' => t('Edit Concert Metadata'),
        '#content' => $form,
        '#tab_name' => 'edit_metadata'
      );
    }
    
    //Allow addition of Performances to the Concert
    if (user_access('ingest new fedora objects')) {
      
    }
    
    //Allow addition of Lectures/Conferences to the Concert
    if (user_access('ingest new fedora objects')) {
      
    }
    
    return $tabset;
  }
  
  /* TODO:  Finish... Then again, is this even a good idea (instead of the odd theme implementation)?...  Iunno.
  private function getConcertMarkupArray($piece_index = NULL) {
    $title = 'get the title';
    return array(
      '#type' => 'markup',
      '#prefix' => '<div class="islandora_fjm_concert">',
      '#suffix' => '</div>',
      'title' => array(
        '#prefix' => '<h3>',
        '#suffix' => '</h3>',
        '#value' => $title,
      ),
      'top' => array(
      
      ),
      'bottom' => array(
      
      ),
    );
  }*/
}
