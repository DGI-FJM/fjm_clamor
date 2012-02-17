<?php 
/**
 * Template for a person/composer
 */
drupal_set_title(t('Composers'));
?>
<div class="islandora_fjm_person">
  <div class='leftcolumn'>
  <?php
  echo theme('fjm_clamor_imagegallery', $pid);
  ?><div class='performance_table'>
  <h3><?php echo t("Performances in CLAMOR"); ?></h3><?php
  echo theme('table', $performance_headers, $performances['associated']);
  ?></div>
  </div>
  <div class='rightcolumn'>
  <h2><?php echo $name['first'] . " " . $name['last'] ?></h2>
  <h3>
      <?php echo ((!empty($date['birth']) && $date['birth'] !== FALSE) ? ($date['birth']) : t('unknown'))
          . ' - ' . ((!empty($date['death']) && $date['death'] !== FALSE) ? ($date['death']) : ''); ?>
  </h3>
  <div class="fjm_person_bio">
      <?php echo $biography ?>
  </div>

  </div>
</div>