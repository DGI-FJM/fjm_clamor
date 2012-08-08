<?php

abstract class BasicDisplay {
  protected $pid;
  protected $title;
  
  public function __construct($pid) {
    $this->pid = $pid;
  }
  
  public static function get_title($pid) {
    module_load_include('inc', 'fedora_repository', 'api/fedora_item');
    $item = new Fedora_Item($pid);
    return $item->objectProfile->objLabel;
  }
  
  public static function addTabsStatically($pid, $page_number = NULL) {
    $tabset = array();
    
    $tabset['test'] = array(
      '#value' => '<span>This display still needs to be completed.</span>',
    );
    
    return $tabset;
  }
  
  public function addTabs($page = NULL) {
    return static::addTabsStatically($this->pid, $page);
  }
  
  protected function getImageTab($pid, $weight = NULL) {
    return array(
      'manage-images' => array(
        '#type' => 'tabpage',
        '#title' => t('Manage Images'),
        '#weight' => $weight,
        '#content' => drupal_get_form('fjm_clamor_image_set_form', $pid),
      ),
    );
  }
}

function fjm_clamor_image_set_form(&$form_state, $pid) {
  module_load_include('inc', 'php_lib', 'Ahah');
  Ahah::get_ahah_js();
  
  $form = array(
    '#tree' => TRUE,
    '#action' => '#manage-images',
    'pid' => array(
      '#type' => 'value',
      '#value' => $pid,
    ),
    'image_table' => array(
      '#type' => 'fieldset',
      '#title' => t('Associated Images'),
      '#description' => t('The images associated with this asset.'),
      '#theme' => 'fjm_clamor_image_manage_table',
    ),
    'add' => array(
      '#prefix' => '<div id="image-add-wrapper">',
      '#suffix' => '</div>',
      '#type' => 'fieldset',
      '#title' => t('Add image'),
      'method' => array(
        '#type' => 'select',
        '#title' => t('Method'),
        '#description' => t('Where the new image is to come from:  Whether it\'s an existing image we will just link to, or a new image object we will create.'),
        '#options' => array(
          'existing' => t('Existing'),
          'new' => t('New'),
        ),
        '#default_value' => (isset($form_state['post']['add']['method']) ? 
          $form_state['post']['add']['method']:
          'existing'),
        '#ahah' => array(
          'path' => 'fjm_clamor/image_addition_change',
          'wrapper' => 'image-add-wrapper',
        ),
      ),
      'add_existing' => array(
        '#access' => (isset($form_state['post']['add']['method']) ? 
          ($form_state['post']['add']['method'] == 'existing'):
          (TRUE)),
        '#type' => 'fieldset',
        '#title' => t('Link an existing image'),
        '#description' => t('Select an image already exising in the repository to link.'),
        'to_link' => array(
          '#type' => 'textfield',
          '#title' => t('PID of item to link'),
          '#description' => t('TODO:  Allow use of autocomplete using description...'),
        ),
        'make_icon' => array(
          '#type' => 'checkbox',
          '#title' => t('Make icon'),
          '#description' => t('Make this image the icon for this object.')
        ),
        'create_link' => array(
          '#type' => 'submit',
          '#name' => 'create_link',
          '#value' => t('Create link'),
        ),
      ),
      'add_new' => array(
        '#access' => (isset($form_state['post']['add']['method']) ? 
          ($form_state['post']['add']['method'] == 'new'):
          (FALSE)),
        '#type' => 'fieldset',
        '#title' => t('Add new image'),
        '#description' => t('Give an image file and a description wherewith to ' .
          'create (and associate) a new image object.'),
        'file' => array(
          '#tree' => FALSE,
          '#type' => 'upload_element',
          '#title' => t('Image file'),
          '#description' => t('The image file.'),
        ),
        'description' => array(
          '#type' => 'textarea',
          '#title' => t('Description'),
          '#description' => t('A description of the contents of the image.'),
        ),
        'make_icon' => array(
          '#type' => 'checkbox',
          '#title' => t('Make icon'),
          '#description' => t('Make this image the icon for this object.')
        ),
        'create_new' => array(
          '#type' => 'submit',
          '#name' => 'create_new',
          '#value' => t('Create New'),
        ),
      ),
    ),
  );
  
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  $results = ObjectHelper::performSparqlQuery(<<<EOQ
PREFIX fm: <info:fedora/fedora-system:def/model#>
PREFIX atm-rel: <http://digital.march.es/atmusica#>
SELECT ?img ?is_icon ?desc
WHERE {
  ?img atm-rel:isImageOf <info:fedora/$pid> ;
       fm:state fm:Active .
  OPTIONAL {
    ?img <dc:description> ?desc .   
  }
  OPTIONAL {
    ?img atm-rel:isIconOf ?is_icon .
    FILTER(sameTerm(?is_icon, <info:fedora/$pid>)) .
  }
}
EOQ
);
  foreach ($results as $key => $result) {
    $img_pid = $result['img'];
    $asdf = form_clean_id("desc-wrapper-fewq-$key");
    $row = array(
      '#type' => 'item',
      'image' => array(
        '#type' => 'value',
        '#value' => $img_pid,
      ),
      'description' => array(
        '#prefix' => '<div id="' . $asdf . '">',
        '#suffix' => '</div>',
        '#type' => 'textarea',
        '#attributes' => array(
          'pid' => $img_pid,
        ),
        '#default_value' => $result['desc'],
        '#ahah' => array(
          'path' => 'fjm_clamor/image_description/' . $key,
          'wrapper' => $asdf,
          'event' => 'change', //change = Change + blur, instead of just blur?
        ),
      ),
      'iconify' => array(
        '#type' => 'submit',
        '#attributes' => array(
          'pid' => $img_pid,
        ),
        '#name' => 'iconify--' . $img_pid,
        '#value' => t('Make Icon'),
      ),
      'disassociate' => array(
        '#type' => 'submit',
        '#name' => 'disassociate--' . $img_pid,
        '#attributes' => array(
          'pid' => $img_pid,
        ),
        '#value' => t('Delete image association'),
      ),
    );
    if (!!$result['is_icon']) {
      $row['iconify'] = array(
        '#value' => t('Is icon'),
      );
      $row['disassociate'] = array(
        '#value' => t('Cannot disassociate icons.  Make another image an icon to remove this one.')
      );
    }
    $form['image_table'][$key] = $row;
  }
  return $form;
}


function fjm_clamor_image_set_form_validate($form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_utils');
  
  if ($form_state['clicked_button']['#name'] == 'create_link' && 
    (empty($form_state['values']['add_existing']['to_link']) ||
    valid_pid($form_state['values']['add_existing']['to_link']))) {
    form_set_error(implode($form['add_existing']['to_link']['#parents'], ']['), t('Need to specify a valid PID!'));
  }
  elseif ($form_state['clicked_button']['#name'] == 'create_new' &&
    empty($form_state['values']['file'])) {
    form_set_error(implode($form['add_new']['file']['#parents'], ']['), t('Need to give an image file!'));
  }
}

function fjm_clamor_image_set_form_submit(&$form, &$form_state) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  module_load_include('inc', 'fedora_repository', 'ObjectHelper');
  
  $pid = $form_state['values']['pid'];
  $atm_rel = 'http://digital.march.es/atmusica#';
  $item = new Fedora_Item($pid);
  
  $img = NULL;
  $make_icon = FALSE;
  $disassociate = FALSE;
  if ($form_state['clicked_button']['#name'] == 'create_link') {
    $img = new Fedora_Item($form_state['values']['add']['add_existing']['to_link']);
    $make_icon = $form_state['values']['add']['add_existing']['make_icon'];
  }
  elseif ($form_state['clicked_button']['#name'] == 'create_new') {
    $new_pid = Fedora_Item::get_next_PID_in_namespace('atm-test');
    $desc = trim($form_state['values']['add']['add_new']['description']);
    $make_icon = $form_state['values']['add']['add_new']['make_icon'];
    $img = Fedora_Item::ingest_new_item($new_pid, 'A', truncate_utf8($desc, 255, TRUE, TRUE));
    sleep(3); //XXX:  ObjectLockedException...  Maybe this'll help?
    if ($desc) {
      fjm_clamor_set_dc_description($img->pid, $desc);
    }
    
    $img->add_datastream_from_file($form_state['values']['file']->filepath,
      'JPG',
      'Object image',
      $form_state['values']['file']->filemime);
    $img->add_relationship('hasModel', 'atm:imageCModel', FEDORA_MODEL_URI);
  }
  elseif (strpos($form_state['clicked_button']['#name'], 'iconify') === 0) {
    $img = new Fedora_Item($form_state['clicked_button']['#attributes']['pid']);
    $make_icon = TRUE;
  }
  elseif (strpos($form_state['clicked_button']['#name'], 'disassociate') === 0) {
    $img = new Fedora_Item($form_state['clicked_button']['#attributes']['pid']);
    $disassociate = TRUE;
  }
  
  if ($img !== NULL) {
    $this_uri = "info:fedora/$pid";
    if ($disassociate) {
      $img->purge_relationships('isImageOf', $this_uri, $atm_rel);
    }
    else {
      $img->add_relationship('isImageOf', $pid, $atm_rel);
      if ($make_icon) {
        $results = ObjectHelper::performSparqlQuery(<<<EOQ
PREFIX atm-rel: <$atm_rel>
SELECT ?obj
WHERE {
  ?obj atm-rel:isIconOf <$this_uri> .
}
EOQ
        );
        
        foreach ($results as $result) {
          $unassoc = new Fedora_Item($result['obj']);
          $unassoc->purge_relationships('isIconOf', $this_uri, $atm_rel);
        }
      
        $img->add_relationship('isIconOf', $pid, $atm_rel);
      }
    }
  }
}

function theme_fjm_clamor_image_manage_table($element) {
  //dsm($element, 'table theme args');
  $rows = array();
  foreach (element_children($element) as $key) {
    $el = $element[$key];
    $row = array(
      fedora_repository_render_image($el['image']['#value'], 'TN'),
      drupal_render($el['description']),
    );
    
    if ($el['icon']['#default_value']) {
      $row[] = t('Is Object Icon');
    }
    else {
      $row[] = drupal_render($el['iconify']);
      $row[] = drupal_render($el['disassociate']);
    }
    
    $rows[] = $row;
  }
  
  if (count($rows) > 0) {
    $header = array(t('Image'), t('Description'), t('Iconify'), t('Unassociate'));
    return theme('table', $header, $rows);
  }
  else {
    $empty_item = array(
      '#type' => 'item',
      '#title' => t('Images'),
      '#value' => t('No associated images.'),
    ); 
    return drupal_render($empty_item);
  }
}

function fjm_clamor_image_autocomplete($string) {
  //Need to use Solr to find ImageCModel stuff...
  module_load_include('inc', 'islandora_solr_search', 'IslandoraSolrQueryProcessor');
  $qp = new IslandoraSolrQueryProcessor();
}

function fjm_clamor_set_dc_description($pid, $description) {
  module_load_include('inc', 'fedora_repository', 'api/fedora_item');
  
  $item = new Fedora_Item($pid);
  $dc_str = trim($item->get_datastream_dissemination('DC'));
  $dc_doc = DOMDocument::loadXML($dc_str);
  if($dc_doc) {
    //Remove all descriptions.
    foreach ($dc_doc->getElementsByTagName('description') as $d_el) {
      $d_el->parentNode->removeChild($d_el);
    }
    //Add in the single description.
    $desc_el = $dc_doc->createElementNS('http://purl.org/dc/elements/1.1/', 'dc:description', $description);
    $dc_doc->documentElement->appendChild($desc_el);
    $dc_str_changed = $dc_doc->saveXML();
    return $item->modify_datastream($dc_str_changed, 'DC', 'Dublin Core Metadata', 'text/xml');
  }
  else {
    drupal_set_message('Error loading DC.');
  }
}

function fjm_clamor_update_image_description($key) {
  module_load_include('inc', 'php_lib', 'Ahah');
  Ahah::get_ahah_js();
  list($form_id, $build_id, $form, $form_state) = Ahah::getFormInfo();
  Ahah::processForm($form_id, $form, $form_state);
  
  $img_pid = $form['image_table'][$key]['description']['#attributes']['pid'];
  $ret =fjm_clamor_set_dc_description($img_pid, 
    $form_state['values']['image_table'][$key]['description']);
  if ($ret !== NULL) {
    drupal_set_message(t('Modified description for image with PID %id. (Will need to reload to make see the changes in the "view" tab.)', array('%id' => $img_pid)));
  }

  sleep(1);//Ensure the entry in the RI gets updated?
  Ahah::rebuildForm($form_id, $build_id, $form, $form_state);
  
  Ahah::respond($form['image_table'][$key]['description']);
  exit();
}

function fjm_clamor_image_addition_change() {
  module_load_include('inc', 'php_lib', 'Ahah');
  
  list($form_id, $build_id, $form, $form_state) = Ahah::getFormInfo();
  Ahah::rebuildForm($form_id, $build_id, $form, $form_state);
  
  Ahah::respond($form['add']);
  exit();
}