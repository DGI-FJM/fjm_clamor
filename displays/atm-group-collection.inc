<?php

module_load_include('inc', 'fjm_clamor', 'displays/basic-display');

class GroupCollectionDisplay extends BasicDisplay {
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
  ?object fm:hasModel <info:fedora/atm:groupCModel> ;
          fm:state fm:Active ;
          dc:title ?title .
  OPTIONAL {
  ?thumbnail atm-rel:isIconOf ?object ;
             fm:state fm:Active .
  }
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
      $tabset['add-instrument'] = array(
        '#type' => 'tabpage',
        '#title' => t('Add'),
        '#tab_name' => 'add-group',
        '#content' => drupal_get_form('fedora_repository_ingest_form', $pid, self::get_title($pid), 'atm:groupCModel'),
      );
    }

    return $tabset;
  }
}