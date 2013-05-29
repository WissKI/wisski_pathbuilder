<?php
/**
 * @file
 * The Wisski Pathbuilder Module. With this tool the administrator may build
 * "paths" through the ontology beginning with certain concepts and ending
 * with primitive values. This can be seen as a shortcut. Whenever one fills
 * a path, the whole path is created and the primitive value given is filled
 * at the end of the path. 
 */
/*
function wisski_pathbuilder_node_form($node, $form_state) {
  drupal_set_message("Hello!");
}
*/
/**
 * Returns the treetypes for the pathbuilder. 
 * @return An array with key = pathnumber and value = pathname
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getPathbuilderTrees() {
  return array(2 => "Internal display", 3 => "Import", 5 => "Export");
}

/**
 * Retrieves the name-string of a path with the given id. 
 * @param id the id of a path. if the id is invalid, the empty string will be returned.
 * @return the name of the path or the empty string.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getName($id) {
  $result = db_query("SELECT * FROM {wisski_template} WHERE id = '$id'");
  $arr = db_fetch_array($result);
  
  if(!isset($arr["name"])) return "";
  return $arr["name"];
}

/**
 * Retrieves the parent group of a path with the given id.
 * @param id the id of a path. if the id is invalid, -1 will be returned.
 * @return the id of the parent group or -1.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getParentGroup($id) {
  $result = db_query("SELECT * FROM {wisski_template} WHERE id = '$id'");
  $arr = db_fetch_array($result);
  
  if(!isset($arr["group_id"])) return -1;
  return $arr["group_id"];
}

function _wisski_pathbuilder_debug($message = NULL, $var = NULL) {
  $debug_backtrace = debug_backtrace();
  $prefix = "DEBUG "
  . $debug_backtrace[1]['function']                    // calling function
  . " (line " . $debug_backtrace[1]['line'] . "): ";   // line number of call

  if( module_exists('devel')) {
    if($var != NULL) dpm($var, $prefix . $message);
    else if($message != NULL) drupal_set_message($prefix . $message);
  }
}

/**
 * Returns all ids of paths from the database that are groups.
 * @param type the type of the group, default: all
 * @return an array of ids of paths that are groups. Not the paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getGroups($type = 1) {

  $result = db_query("SELECT * FROM {wisski_template} WHERE is_group = 1");
  $groupids = array();
  
  while($arr = db_fetch_array($result)) {
    if( ($arr["type"] % $type) == 0) 
      $groupids[] = $arr["id"];
  }
  
  return $groupids;
}

/**
 * Returns all ids of paths from the database that are interna-groups.
 * 
 * @return an array of ids of paths that are interna-groups. Not the paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getInternaGroups() {
  $groupids = wisski_pathbuilder_getGroups(2);
  
  return $groupids;
}

/**
 * Returns all ids of paths from the database that are groups and enabled.
 * 
 * @return an array of ids of paths that are groups. Not the paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getEnabledGroups($type = 1) {
  
  $result = db_query("SELECT * FROM {wisski_template} WHERE is_group = 1 AND enabled = 1"); 
  $groupids = array();
  
  while($arr = db_fetch_array($result)) {
    if( ($arr["type"] % $type) == 0)
      $groupids[] = $arr["id"];
  }
  
  return $groupids;
}

/**
 * Returns all ids of paths from the database that are interna-groups and enabled.
 * 
 * @return an array of ids of paths that are groups. Not the paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getEnabledInternaGroups() {
  $groupids =  wisski_pathbuilder_getEnabledGroups(2);
    
  return $groupids;
}

/**
 * Returns all ids of paths from the database that are groups for a certain individual or concept.
 * 
 * @return an array of ids of paths that are groups. Not the paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getGroupsForConcept($con, $is_individual = TRUE, $type = 1) {

  if(empty($con))
    return null;
  
  $result = db_query("SELECT * FROM {wisski_template} WHERE is_group = 1");

  $groupids = array();
  
  $mycon = $con;

  if($is_individual) {  
    $q = "SELECT ?z WHERE { <$con> rdf:type ?z }";
    
    if($rows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q, 'rows')) {
      $mycon = $rows[0]['z'];
    }
  }
  
  while($arr = db_fetch_array($result)) {
    $samepart = _wisski_pathbuilder_calculate_group_samepart($arr['id']);
    if($samepart['x' . floor(count($samepart)/2)] == $mycon) { 
      if( ($arr["type"] % $type) == 0)
        $groupids[] = $arr["id"];
    }
  }
  
  return $groupids;
}

/**
 * Returns all ids of paths from the database that are interna-groups for a certain individual or concept.
 * 
 * @return an array of ids of paths that are groups. Not the paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getInternaGroupsForConcept($con, $is_individual = TRUE) {
  $groupids = wisski_pathbuilder_getGroupsForConcept($con, $is_individual, 2);
  
  return $groupids;
}

/**
 * Returns all ids of paths from the database that are no groups.
 * 
 * @return an array of ids of paths that are no groups. Just paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getPaths($type = 1) {

  $result = db_query("SELECT * FROM {wisski_template} WHERE is_group = 0");

  $groupids = array();
  
  while($arr = db_fetch_array($result)) {
    if( ($arr["type"] % $type) == 0)
      $groupids[] = $arr["id"];
  }
  
  return $groupids;
}

/**
 * Returns all ids of interna-paths from the database that are no groups.
 * 
 * @return an array of ids of paths that are no groups. Just paths itself.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getInternaPaths() {
  $groupids = wisski_pathbuilder_getPaths(2);
  
  return $groupids;
}

/**
 * Returns all paths that are in a given group. The second parameter tells
 * if subgroups should be included or only true paths should be returned.
 * @param groupid the id of the group which members should be returned.
 * @param include_subgroups TRUE if subgroups should be included, FALSE if not - defaults to FALSE.
 * @return The ids of all paths that are in the given group.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getMembers($groupid, $include_subgroups = FALSE) {
  $result = db_query("SELECT * FROM {wisski_template} WHERE " .
    (($include_subgroups) ? "" : "is_group = 0 AND ") . "group_id = '$groupid'");
  $memberids = array();
  
  while($arr = db_fetch_array($result)) {
    $memberids[] = $arr["id"];
  }
  
  return $memberids;
}

/**
 * Returns all paths that are in a given group and enabled and internal. The second parameter tells
 * if subgroups should be included or only true paths should be returned.
 * @param groupid the id of the group which members should be returned.
 * @param include_subgroups TRUE if subgroups should be included, FALSE if not - defaults to FALSE.
 * @return The ids of all paths that are in the given group.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getEnabledInternaMembers($groupid, $include_subgroups = FALSE, $expand_subgroups = FALSE) {
  $query = "SELECT * FROM {wisski_template} WHERE enabled = 1 AND " .
    (($include_subgroups) ? "" : "is_group = 0 AND ") . "group_id = '$groupid'";
  $query .= " ORDER BY weight ASC";
  $result = db_query($query);
  $memberids = array();
  
  while($arr = db_fetch_array($result)) {
    if( ($arr["type"] % 2) == 0)
      if($expand_subgroups == FALSE)
        $memberids[] = $arr["id"];
      else {
        if($arr['is_group'] == 1)
          $memberids = array_merge($memberids, wisski_pathbuilder_getEnabledInternaMembers($arr['id'], $include_subgroups, $expand_subgroups));
        else
          $memberids[] = $arr["id"];
      }    
      
      
    }
    
  
  return $memberids;
}

function wisski_pathbuilder_getAllMembers($groupid, $include_subgroups = FALSE) {
  $memberids = array();
  $curId = $groupid;
  $ids = array();

  while($curId && $arr = wisski_pathbuilder_getMembers($curId, TRUE)) {

    foreach($arr as $key => $value) {
      $path_data = wisski_pathbuilder_getPathData($value);
      if ($path_data['is_group'] == 0) {
        $memberids[] = $value;
      } else {
        if($include_subgroups == TRUE) $memberids[] = $value;  
        $ids[] = $value;
      }
    }
    $curId = array_pop($ids);
  }  
  
  return $memberids;
  
}

/**
 * Get all data for a path with a given pathid from the database. 
 * @param pathid a valid id of a path.
 * @return an array in the format of the database.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getPathData($pathid) {
  $result = db_query("SELECT * FROM {wisski_template} WHERE id = '$pathid'");
    
  $arr = db_fetch_array($result);
 
  return $arr;     
}

/**
 * Get an array of offset numbers where the path is in the form array
 * @param pathid the id of the path
 * @return an array of offsets
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getFormOffset($pathid) {
  $data =  wisski_pathbuilder_getPathData($pathid);
  
  if($data['group_id'] == 0) 
    return array($pathid);
    
  $out = wisski_pathbuilder_getFormOffset($data['group_id']);
  
  $out[] = $pathid;
  
  return $out;
  
}

/**
 * Save the given parameters to the database.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_saveToDB($path_array, $length, $name, $description, $groupid, $weight, $isgroup, $fieldtype, $enabled, $mandatory, $repeatable, $type, $uuid = NULL) {

  if(empty($uuid)) {
    $uuid = wisski_get_uid();
  }
  db_query("INSERT INTO {wisski_template} 
  (uuid, path_array, length, name, description, group_id, weight, is_group, fieldtype, enabled, mandatory, repeatable, type) 
  VALUES 
  ('%s', '%s', %d, '%s', '%s', %d, %d, %d, '%s', %d, %d, %d, %d)", 
  $uuid, $path_array, $length, $name, $description, $groupid, $weight, $isgroup, $fieldtype, $enabled, $mandatory, $repeatable, $type);

}

/**
 * Get the option array for various fieldtypes e.g. dropdown etc. Moreover
 * the options for auto completion are generated here, too.
 * @param pathid a valid id of a path for which the options should be generated
 * @param $arg2 a string all options have to include. Omitted for no string.
 * @return an array of options with key = value.
 * @author Mark Fichtner, Martin Scholz
 */
function wisski_pathbuilder_getOptionArray($pathid, $arg2 = FALSE) {

  drupal_add_js( 'sites/all/modules/wisski_pathbuilder/wisski_pathbuilder.js');

  $out = array();
  $data = wisski_pathbuilder_getPathData($pathid);
  $path_array = unserialize($data['path_array']);
  $pgroup_path = _wisski_pathbuilder_calculate_group_samepart($data['group_id']);
  $min_x = (count($pgroup_path) + 1) / 2; // common path of containing group plus one concept
  $min_x = max($data['disamb'] - 1, $min_x);

  if (module_exists('wisski_vocab_ctrl')) {
    drupal_load('module', 'wisski_vocab_ctrl');
    $auths = wisski_vocab_ctrl_get();
  } else {
    $auths = array();
  }
  $sortauths = array();
  
  foreach($auths as $auth) {
    if($auth->accesspoint->type == "local") continue;

    $group_id = $auth->group_id;
    $group_path = _wisski_pathbuilder_calculate_group_samepart($group_id);
    $concept = $group_path['x0'];

    if($key = array_search($concept, $path_array)) {
      $key = substr($key, 1);
      if($key >= $min_x) {
        $sortauths[$key][] = $auth;
      } 
    }
    
  }
  
  $h_split = explode('|', $arg2, 2);
  if (isset($h_split[1]) && trim($h_split[0])) {
    $h_term = trim($h_split[0]);
    $h_part = trim($h_split[1]);
  } else {
    $h_term = NULL;
  }

  $limit = 15;
  $offset = 0;

//  drupal_set_message(serialize($sortauths));
//  return;

  krsort($sortauths);
  
  foreach ($sortauths as $k => $auths) {

    foreach($auths as $auth) {
        
        $local_limit = $limit - count($out);
        if ($h_term) {
          $uris = wisski_vocab_ctrl_lookup('labels', $auth, NULL, $h_term, $local_limit, $offset, 'exact', 'normal', TRUE);
        } else {
          $uris = wisski_vocab_ctrl_lookup('labels', $auth, NULL, $arg2, $local_limit, $offset, 'starts', 'normal', TRUE);  // viel schneller!
 //         $uris = wisski_vocab_ctrl_lookup('labels', $auth, NULL, $arg2, $local_limit, $offset, 'contains');
        }
//return;        
        if (!$uris) {
          continue;
        }

        foreach($uris as $individual => $labels) {
          $preflabel = NULL;
          $altlabel = NULL;
          if (isset($labels['label'])) {
            $label = $labels['label'][0]['value'];
            if (isset($labels['alt_label'])) {
              $altlabel = $labels['alt_label'][0]['value'];
            } else {
              $l_array = wisski_vocab_ctrl_lookup('alt_label', $auth, $individual, NULL, NULL, NULL, 'exact', 'normal', TRUE);
              if (!empty($l_array) && isset($l_array[$individual])) {
                $altlabel = $l_array[$individual]['alt_label'][0]['value'];
              }
            }

          } elseif (isset($labels['alt_label'])) {
            $label = $labels['alt_label'][0]['value'];
            $l_array = wisski_vocab_ctrl_lookup('label', $auth, $individual, NULL, NULL, NULL, 'exact', 'normal', TRUE);
            if (!empty($l_array) && isset($l_array[$individual])) {
              $preflabel = $l_array[$individual]['label'][0]['value'];
            }
          } else {
            $ks = array_keys($labels);
            $label = $labels[$ks[0]][0]['value'];
          }

          $broaders_array = wisski_vocab_ctrl_lookup('broader_labels', $auth, $individual, NULL, NULL, NULL, 'exact', 'normal', TRUE);
          if ($broaders_array) $broaders = join(', ', $broaders_array[$individual]['broader_labels']);
          
          if ($h_term) {
            $newlabel = '';
            foreach ($broaders_array[$individual]['broader_labels'] as $b) {
              if (mb_strpos(mb_strtolower($b), mb_strtolower($h_part)) !== FALSE) {
                $newlabel = $label . ', ' . $b;
                break;
              }
            }
            if (!$newlabel) {
              continue; // a hierarchy part is given and it does not match!
            } else {
              $label = $newlabel;
            }
          }
          
          $addedlabel = ($preflabel !== NULL) ? check_plain($preflabel) : (($altlabel !== NULL) ? check_plain($altlabel) : '');
          if (!empty($addedlabel)) $addedlabel = t(' (!l)', array('!l' => $addedlabel));

          if ($h_term && ($a = mb_strpos(mb_strtolower($label), mb_strtolower($h_term))) !== FALSE) {
            $l = mb_strlen($h_term);
            $ap = mb_strpos(mb_strtolower($label), mb_strtolower($h_part));
            $lp = mb_strlen($h_part);
            $termout = check_plain(mb_substr($label, 0, $a)) . '<strong>' . check_plain(mb_substr($label, $a, $l)) . '</strong>' . check_plain(mb_substr($label, $a + $l, $ap - $a - $l)) . '<strong>' .check_plain( mb_substr($label, $ap, $lp)) . '</strong>' . check_plain(mb_substr($label, $ap + $lp));
          }elseif ($arg2 && ($a = mb_strpos(mb_strtolower($label), mb_strtolower($arg2))) !== FALSE) {
            $e = $a + mb_strlen($arg2);
            $termout = check_plain(mb_substr($label, 0, $a)) . '<strong>' . check_plain(mb_substr($label, $a, $e - $a)) . '</strong>' . check_plain(mb_substr($label, $e));
          } else {
            $termout = check_plain($label);
          }

          $outlabel  = '';
          if ($arg2) $outlabel .= '<span title="' . check_plain($label) . ' -- ' . t('from') . ' ' . check_plain($auth->name) . (($broaders) ? check_plain(" ($broaders)") : "") . '">';
          $outlabel .= $termout . $addedlabel . ' -- ' . t('from') . ' ' . check_plain($auth->name) . (($broaders) ? check_plain(" ($broaders)") : "");
          if ($arg2) $outlabel .= '</span>';
          $out['[vid:'. $auth->vid . '] [ind:' . $individual . '] [x:' . $k . '] ' . $label] = $outlabel;
        
          if(count($out) >= $limit) {
            break;
          }
        }
    }

    if(count($out) >= $limit) {
      break;
    }

  }

  if(empty($out)) {
    $result = db_query("SELECT * FROM {wisski_template} WHERE id = $pathid");
    $row = db_fetch_array($result);
   
    if ($arg2) {
      $arg2_preg_quote = wisski_store_escape_sparql_literal(wisski_store_escape_sparql_regex($arg2));
    } 
   
    if($row['disamb'] > 1) {
      if($arg2)
        $sparql = wisski_pathbuilder_get_sparql($pathid, 'FILTER regex(?out, "' . $arg2_preg_quote . '", "i" )', FALSE, $row['disamb'] - 1, FALSE);
      else
        $sparql = wisski_pathbuilder_get_sparql($pathid, "", FALSE, $row['disamb'] - 1);
    } else {
      if($arg2)
        $sparql = wisski_pathbuilder_get_sparql($pathid, 'FILTER regex(?out, "' . $arg2_preg_quote . '", "i" )', FALSE, 0, FALSE);
      else
        $sparql = wisski_pathbuilder_get_sparql($pathid, "");
    }
    
    $sprows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql, 'rows');
  
    foreach($sprows as $row) {
      if($arg2)
        $out[$row['out']] = str_ireplace($arg2, '<strong>' . $arg2 . '</strong>', $row['out']);
      else
        $out[$row['out']] = $row['out'];
    }
  }
  natsort($out);

  return $out;
}



/**
 * The function that handles the autocompletion magic in form fields.
 * Gets two parameters, the path that should be autocompleted and the 
 * already written string.
 * @param pathid the path that should be autocompleted
 * @param arg2 the already typed string.
 * @return the json sniplet for auto completion.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_autocomplete($pathid = 1, $arg2 = '') {
  drupal_set_header('Content-Type: text/html;');
  // Dont autocomplete if less than two chars.
  if(strlen($arg2) < 2) { 
    echo drupal_to_js(array());
    return;
  }

  $out = wisski_pathbuilder_getOptionArray($pathid, $arg2);

  $additional = "<em>" . t("Continue typing for more hits") . ".</em>";

  if(count($out) > 15) {
    $out = array_slice($out, 0, 15, TRUE); 
    $out[''] = $additional;
  }
  echo drupal_to_js($out);

}

function wisski_pathbuilder_duplicate($pathid = 1) {

//  drupal_set_header('Content-Type: text/plain; charset=utf-8');
  
//  drupal_set_message(serialize(drupal_get_headers()));
  
//  return; 

  module_load_include('inc', 'node', 'node.pages');
  module_load_include('module', 'wisski_pathbuilder', 'wisski_pathbuilder');
  $form_state = array('storage' => NULL, 'submitted' => FALSE, 'values' => $_POST);

  $form_build_id = $_POST['form_build_id'];
  $form = form_get_cache($form_build_id, $form_state);

//  drupal_set_message("hahaha!");
//  drupal_set_message(serialize($pathid));

  $args = $form['#parameters'];
  $form_id = array_shift($args);
  $form_state['post'] = $form['#post'] = $_POST;

  $form['#programmed'] = $form['#redirect'] = FALSE;

//  drupal_set_message(serialize($form_state));
//  return;


  drupal_process_form($form_id, $form, $form_state);
  
//  drupal_set_message(serialize($form_state));
  
  $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

//  drupal_set_message(serialize($form_state));

//  $offset = wisski_pathbuilder_getFormOffset($pathid);
    $offset = $form_state['clicked_button']['#parents'];
//  drupal_set_message("looking for offset: " . serialize($form_state));

//  drupal_set_message("clicked button is: " . serialize($form_state['clicked_button']));

  $out = $form; //form_get_cache($form_build_id, $form_state);
//  drupal_set_message("out: " . serialize($out));
  $out = $out['form'];
  $lastOff = "";
  
  unset($offset[count($offset)-1]);
  unset($offset[count($offset)-1]);
//  drupal_set_message("offset is: " . serialize($offset));
  
  $offset[] = $offset[count($offset)-1];
  
//  drupal_set_message("starting with " . serialize($out));
  foreach($offset as $off) {
    $out = $out[$off];
//    $lastOff = $off;
//    drupal_set_message("subtree of " . serialize($off) . " is: " . serialize($out));
//    foreach($out as $key => $v)
//      drupal_set_message("subkey could be: $key");
  }

//  if($lastOff != "")
//    $out = $out[$lastOff];

//  drupal_set_message(serialize($out));

  unset($out['#prefix'], $out['#suffix']);
  
//  drupal_set_message(serialize($out));
  
//  drupal_set_message(drupal_render($out));

  drupal_set_header('Content-Type: text/html; charset=utf-8');

  //drupal_json(array(
  echo drupal_to_js(array(
        'status'   => TRUE,
        'data'     => theme('status_messages') . drupal_render($out),
        ));

  
}

function wisski_pathbuilder_filter_auth_data($array) {
  $newarray = array();
  foreach($array as $key => $value)
    if(strpos($key, 'auth_data') === FALSE)
      $newarray[$key] = $value;
  return $newarray;
}

function wisski_pathbuilder_dupl_submit($form, &$form_state) {
//  drupal_set_message("I'm first!","error");
//  wisski_pathbuilder_rescue_values($form, $form_state);
  $arr = explode('/', $form_state['clicked_button']['#ahah']['path']);
  //$pathid = $arr[count($arr)-1];

  //drupal_set_message(serialize(count($form_state['values'][1][1][6][6])));

//  drupal_set_message("fs is: " . serialize($form_state));

  //$offset = wisski_pathbuilder_getFormOffset($pathid);
  //$form_state['wisskiValues'] = $form_state['values'];
  $offset = $form_state['clicked_button']['#parents'];
//  drupal_set_message("klicked button is: " . serialize($form_state['clicked_button']));
  $form_state['rebuild'] = TRUE;
  
  unset($offset[count($offset)-1]);
  unset($offset[count($offset)-1]);

  $offset[] = $offset[count($offset)-1];
  $offset = array_values($offset);
//  $offset[] = $offset[count($offset)-1] . '_' . 
  $tmpval = &$form_state['values'];
  
//  drupal_set_message(serialize($offset));
  
  foreach($offset as $off) {
//    drupal_set_message("I'm currently here: " . serialize($tmpval), 'error');
//    drupal_set_message("I'm trying it with " . $off, 'error');
    $tmpval = &$tmpval[$off];
  }

//  drupal_set_message(serialize($tmpval));
  
//  drupal_set_message(serialize($offset));
  //drupal_set_message(serialize($tmpval));

  $myval = wisski_pathbuilder_filter_auth_data($tmpval);
  
//  drupal_set_message(serialize($tmpval));  
  
  $newoffset = $offset[count($offset)-1] . '_' . count($myval);
  
//  drupal_set_message("new offset is: " . $newoffset);
  
  if(strpos($form_state['clicked_button']['#ahah']['wrapper'], "wisski_pathbuilder_ontogroup_wrapper") !== FALSE)
    $tmpval[$newoffset] = array();
  else
    $tmpval[$newoffset] = "";
  //$form_state['wisskiValues'][$pathid][$pathid . "_" . count($form_state['wisskiValues'][$pathid])] = "";
  //drupal_set_message("dupl: " . serialize($form_state));
  //$form_state['rebuild'] = TRUE;
  wisski_pathbuilder_rescue_values($form, $form_state);
}

/**
 * Get the groupid for one type.
 * @param type the type 
 * @return the id of the group for the user.
 * @author Mark Fichtner
 */ 
function wisski_pathbuilder_getGroupIDForConcept($type, $use_type = 1) {

  if(!$type || !isset($type)) {
    drupal_set_message("no type supplied!","error");
    return;
  }

  // Get the favorite user groups for all users
  $var = variable_get("wisski_pathbuilder_templateids", array());
  //drupal_set_message("var: " . serialize($var));

  // get the user
  global $user;

  // get all groups in the system
  $groups = wisski_pathbuilder_getEnabledGroups($use_type);


  // if the user has a favorite type get that type and return it.
  if($var && isset($var) && $var[$user->uid] && isset($var[$user->uid]) && $var[$user->uid][$type] && in_array($var[$user->uid][$type], $groups))
    return $var[$user->uid][$type];

  // oh... no group was defined till now... take the first one found.
  foreach($groups as $group) {
    $samepart = _wisski_pathbuilder_calculate_group_samepart($group);
    // is this my group?  
    if(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($samepart[("x" . (floor(count($samepart)/2)))]) == $type) {
      return $group;
    }
  }
  return -1;
}

/**
 * Get the groupid for one type.
 * @param type the type 
 * @return the id of the group for the user or -1 if there is none.
 * @author Mark Fichtner
 */ 
function wisski_pathbuilder_getInternaGroupIDForConcept($type) {
  return wisski_pathbuilder_getGroupIDForConcept($type, 2);
}

/**
 * Get the groupid for an individual.
 * @param ind the individual with its long name.
 * @return the id of the group for the user.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getGroupIDForIndividual($ind, $type = 1) {

  if(!$ind || empty($ind)) {
    drupal_set_message("no individual supplied.", "error");
    return;
  }

  $ind = str_replace(" ", "_", $ind);

  $sparql = "SELECT ?x WHERE { <" . $ind . "> <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type") . "> ?x }";
  $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql);

  $results = $out['result']['rows'];

  // got nothing? Then this should stop.
  if(!isset($results)) {
    return;
  }

  foreach($results as $result) {
    $id = wisski_pathbuilder_getGroupIDForConcept($result['x'], $type);
    if($id)
      return $id;
  }
  return -1;
}

/**
 * Get the groupid for an individual.
 * @param ind the individual with its long name.
 * @return the id of the group for the user.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_getInternaGroupIDForIndividual($ind) {
  return wisski_pathbuilder_getGroupIDForIndividual($ind, 2);
}

/**
 * The function that generates the display when the edit-button is invoked
 * on any node that has a template for view.
 * @param form_state the current state of the form.
 * @param node the node object that is viewed.
 * @return a form-array for drupal_render.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_template_edit($form_state, $node) { 

  $groupid = wisski_pathbuilder_getGroupIDForIndividual(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title));
  $samepart = _wisski_pathbuilder_calculate_group_samepart($groupid);

  $sparqlcondition = (" FILTER ( ?x" . (floor(count($samepart)/2)) . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title). "> )");

  $fs = FALSE;

  if(!empty($form_state['wisskiValues']))
    $fs = $form_state['wisskiValues'];
  else if(!empty($form_state['values']))
    $fs = $form_state['values'];
  else if(!empty($form_state['post']))
    $fs = $form_state['post'];

//  $fs = unserialize('a:4:{s:7:"storage";N;s:9:"submitted";b:0;s:6:"values";a:15:{i:1;a:1:{i:1;a:18:{i:2;s:2:"13";s:11:"auth_data_2";s:0:"";i:14;s:3:"123";s:12:"auth_data_14";s:0:"";i:3;s:0:"";s:11:"auth_data_3";s:0:"";i:4;s:0:"";s:11:"auth_data_4";s:0:"";i:6;s:4:"null";i:5;s:0:"";s:11:"auth_data_5";s:0:"";s:12:"auth_data_16";s:0:"";i:12;s:0:"";s:12:"auth_data_12";s:0:"";i:13;s:0:"";s:12:"auth_data_13";s:0:"";s:14:"auth_data_15_0";s:0:"";i:15;a:1:{i:15;a:1:{s:4:"15_0";s:57:"http://wisski.gnm.de/dev/sites/default/files/Download.jpg";}}}}s:4:"menu";a:3:{s:10:"link_title";s:0:"";s:6:"parent";s:15:"primary-links:0";s:6:"weight";s:1:"0";}s:5:"group";s:1:"1";s:7:"changed";s:10:"1358444560";s:13:"form_build_id";s:37:"form-8eecbd2eb1226b92192fcc05ab9b7b8a";s:10:"form_token";s:32:"d23d2b059febd2e4fb2b8c106c917ec7";s:7:"form_id";s:20:"individual_node_form";s:3:"log";s:0:"";s:5:"files";a:1:{i:127;a:3:{s:4:"list";s:1:"1";s:11:"description";s:12:"Download.jpg";s:6:"weight";s:1:"0";}}s:7:"comment";s:1:"2";s:4:"path";s:72:"content/ecrm_E84_Information_Carrier394e764e-70be-8dc4-b71b-2c3c1fe56ff1";s:4:"name";s:4:"root";s:4:"date";s:25:"2013-01-14 18:26:50 +0100";s:6:"status";s:1:"1";s:7:"promote";s:1:"1";}s:4:"post";a:15:{i:1;a:1:{i:1;a:18:{i:2;s:2:"13";s:11:"auth_data_2";s:0:"";i:14;s:3:"123";s:12:"auth_data_14";s:0:"";i:3;s:0:"";s:11:"auth_data_3";s:0:"";i:4;s:0:"";s:11:"auth_data_4";s:0:"";i:6;s:4:"null";i:5;s:0:"";s:11:"auth_data_5";s:0:"";s:12:"auth_data_16";s:0:"";i:12;s:0:"";s:12:"auth_data_12";s:0:"";i:13;s:0:"";s:12:"auth_data_13";s:0:"";s:14:"auth_data_15_0";s:0:"";i:15;a:1:{i:15;a:1:{s:4:"15_0";s:57:"http://wisski.gnm.de/dev/sites/default/files/Download.jpg";}}}}s:4:"menu";a:3:{s:10:"link_title";s:0:"";s:6:"parent";s:15:"primary-links:0";s:6:"weight";s:1:"0";}s:5:"group";s:1:"1";s:7:"changed";s:10:"1358444560";s:13:"form_build_id";s:37:"form-8eecbd2eb1226b92192fcc05ab9b7b8a";s:10:"form_token";s:32:"d23d2b059febd2e4fb2b8c106c917ec7";s:7:"form_id";s:20:"individual_node_form";s:3:"log";s:0:"";s:5:"files";a:1:{i:127;a:3:{s:4:"list";s:1:"1";s:11:"description";s:12:"Download.jpg";s:6:"weight";s:1:"0";}}s:7:"comment";s:1:"2";s:4:"path";s:72:"content/ecrm_E84_Information_Carrier394e764e-70be-8dc4-b71b-2c3c1fe56ff1";s:4:"name";s:4:"root";s:4:"date";s:25:"2013-01-14 18:26:50 +0100";s:6:"status";s:1:"1";s:7:"promote";s:1:"1";}}');

//  drupal_set_message("building with fs: " . serialize($fs));
  $paths = array();
  $form = _wisski_pathbuilder_recursive_build_group($fs, $groupid, $paths, 2, $sparqlcondition, 2);
//  drupal_set_message(serialize($paths));
  $form['#cache'] = TRUE;

  $form['group'] = array(
      '#type' => 'hidden',
      '#value' => $groupid,
      );
  
  if(empty($form_state['values']['paths'])) {

  } else {
    $paths = unserialize($form_state['values']['paths']);
  }  
  
//  drupal_set_message(serialize($paths));
  
  $form['paths'] = array(
      '#type' => 'hidden',
      '#value' => serialize($paths),
      );

    
  //drupal_set_message(serialize($paths));
  
  if(empty($form_state['values']['defaults'])) {
    $defaults = wisski_pathbuilder_get_form_state($form);
  } else {
    $defaults = unserialize($form_state['values']['defaults']);
  }

  $form['defaults'] = array(
    '#type' => 'hidden',
    '#value' => serialize($defaults),
  );

/*
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      );
*/

  return $form;
}


/**
 * This function generates the common path part of a group including the
 * already given data (instances) of the current node.
 * @param groupid a valid group
 * @param subjIn the subject which is the starting point for the group
 * @return an array of data with x0, y0, x1 ... xn set to instances.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_get_group_samepart_data($groupid, $subjIn = NULL) {

  if($groupid == 0) {
    drupal_set_message(t("No Data found, error."),'error');
    return;
  }

  $samepart = _wisski_pathbuilder_calculate_group_samepart($groupid);

  if(empty($subjIn)) {
    $arg = arg(1);

    if(empty($arg))
      return;

    $node = node_load(arg(1));

    $types = wisski_store_getObj()->wisski_ARCAdapter_getObjForSubjPred(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title), wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type"));
  } else {
    $types = wisski_store_getObj()->wisski_ARCAdapter_getObjForSubjPred($subjIn, wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type"));
  }
  $pos = -1;

  //drupal_set_message("type: " . serialize($types));
  //drupal_set_message("sp: " . serialize($samepart));
//  drupal_set_message(serialize(debug_backtrace()));

  for($i=0; $i<=(floor(count($samepart)/2)); $i++) {
    foreach($types as $type) {
      if($type == $samepart["x" . $i])
        $pos = $i;
    }
  }

  if($pos == -1) {
    drupal_set_message(t("No Data found 2, error."),'error');
    return;
  }
  if(empty($subjIn))
    $condition = ("FILTER ( ?x" . $pos . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title). "> )");
  else
    $condition = ("FILTER ( ?x" . $pos . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($subjIn). "> )");
    
  $sparql = "";

  $sparql = wisski_pathbuilder_get_sparql($groupid, $condition);

  $sprows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql, 'rows');

//  drupal_set_message("query is: " . htmlentities($sparql));


  $outarr = array();

  if(!$sprows[0]) {
    $result = db_query("SELECT * FROM {wisski_template} WHERE id = $groupid ORDER BY is_group ASC");

    $row = db_fetch_array($result);

    // if nothing was found, search from the group below.
    return wisski_pathbuilder_get_group_samepart_data($row['group_id'], $subjIn);

  }

  for($i=0; $i<count($sprows[0])/2; $i++) {
    $outarr['x' . $i] = $sprows[0]['x' . $i];
  }

  for($i=0; $i<floor(count($samepart)/2); $i++) {
    $outarr['y' . $i] = $samepart["y" . $i];
  }

  return $outarr;


  }

function wisski_pathbuilder_get_form_state($form){
  $values = array();
  _wisski_pathbuilder_form_get_values($form, &$values);
  
//  drupal_set_message(serialize($values));
  $form_state = array();
  $form_state['values'] = $values;
  return ($form_state);
}
          
function _wisski_pathbuilder_form_get_values($element, &$values) {
  foreach (element_children($element) as $key) {
//    drupal_set_message("key is: " . serialize($key));
    if (isset($element[$key]) && $element[$key]) {
      if (isset($element[$key]['#default_value']) && $element[$key]['#default_value']) {
        $values[$key] = $element[$key]['#default_value'];
      } else {
      // Recurse through all children elements.
        _wisski_pathbuilder_form_get_values($element[$key], &$values[$key]);
      }
    }
  }
}

/*
 * @param form_state the current state of the form.
 * @param node the node object that is viewed.
 * @return a form-array for drupal_render.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_template_edit_submit($form, &$form_state) {
//  drupal_set_message("Juhu!");
//  drupal_set_message(serialize($form_state));
  //unset($form_state['storage']);  
  
  //  $form_state['storage']['old_values'] = wisski_pathbuilder_calc_old_values($form['form']);

  $node = node_load(arg(1));
  
  //drupal_set_message("working with node " . serialize($node));
//  drupal_set_message(serialize($form_state));  
//  return;

  $groupid = wisski_pathbuilder_getGroupIDForIndividual(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title));

  $samepart = wisski_pathbuilder_calculate_group_samepart($groupid);

  $condition = ("FILTER ( ?x" . (floor(count($samepart)/2)) . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title). "> )");
//  drupal_set_message($form_state['wisskiValues']['old_values']);
  $old_values = unserialize($form_state['wisskiValues']['old_values']);

  $paths = unserialize($form_state['wisskiValues']['paths']);

  wisski_pathbuilder_recursive_edit_submit($form, $form_state['values'], $groupid, $paths, $condition, $old_values['values']['form']);

  // update title 
  $displaytitle = _wisski_pathbuilder_generateGroupName($node->title, $groupid);
  db_query("UPDATE {wisski_title_cache} SET title = '" . mysql_real_escape_string($displaytitle) . "' WHERE uri = '" . mysql_real_escape_string(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title)) . "'");
  // delete cached image uris
  db_query("DELETE FROM {wisski_image_cache} WHERE uri = '" . mysql_real_escape_string(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title)) . "'");
  
  unset($form_state['storage']);
  $form_state['redirect'] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title);
  
  return;

}

function wisski_pathbuilder_recursive_edit_submit($form, $fs_pointer, $groupid, $paths, $condition, $ov_pointer, $main_subject = "", $parentpath = NULL) {
  //dpm(func_get_args());
  //dpm($fs_pointer);

  $result = db_query("SELECT * FROM {wisski_template} WHERE id = $groupid");
  
  $my_group = db_fetch_array($result);
  
  $node = node_load(arg(1));
  
  if(!empty($node) && !empty($node->nid) && empty($main_subject))
    $main_subject = wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title);
  
  
  $got_triples = array();
  
  $fs_pointer = $fs_pointer[$groupid];
  $ov_pointer = $ov_pointer[$groupid];
  
  //drupal_set_message("fsp is: " . serialize($fs_pointer));
  //drupal_set_message("ovp: " . serialize($ov_pointer));
  
  //if($my_group['fieldtype'] == "ontogroup")
  //  $form_state[
  //else

  $iterator = array($groupid => $groupid);

//  drupal_set_message("fuerr diese gruppe: " . serialize($form_state));
  
//  drupal_set_message("grp: " . serialize($my_group));
  
  if($my_group['fieldtype'] == "ontogroup") {
//    drupal_set_message("yes it is!" . serialize($my_group));
    $fs_pointer = $fs_pointer[$groupid];
    $ov_pointer = $ov_pointer[$groupid];
    $iterator = $fs_pointer;
  }
  
  
  foreach($iterator as $v => $nonsense) {
  
//  drupal_set_message("got: " . $v);
  
    $storefs = $fs_pointer[$v];
    $ov = $ov_pointer[$v];
  
//  drupal_set_message("storefs is: " . serialize($storefs));
  

    $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = $groupid ORDER BY is_group ASC");
    $triples = array();
  

    // go through all fields
    while($row = db_fetch_array($result)) {
      $fs = $storefs;
      $authinfo = array();

      if($row['is_group'] == 1) {
//        drupal_set_message("delegate with condition: " . $condition);
//        drupal_set_message("delegating into " . $row['id'] . " . with fs_pointer . " . serialize($fs_pointer[$v]));
        $got_triples = array_merge($got_triples, wisski_pathbuilder_recursive_edit_submit($form, $fs_pointer[$v], $row['id'], $paths[$v]['children'], $condition, $ov_pointer[$v], $main_subject, $paths[$v]['path']));
         if(empty($main_subject) && empty($condition)) {
           $main_subject = $got_triples[0]['s'];
           $condition = "FILTER ( ?x0 = <" . $main_subject . ">)";
         }
      } else {
    
        if($row['fieldtype'] == 'images' || $row['fieldtype'] == 'ontotext') {
          $fs = $fs[$row['id']];
//          $ov = $ov[$row['id']];  
        }
//      drupal_set_message("Looking for: " . $row['id']);
//      drupal_set_message("fs is: " . serialize($fs));
//      drupal_set_message("fieldtype: " . $row['fieldtype']);    
        if(is_array($fs[$row['id']])) {
          $values = $fs[$row['id']];
          $authinfo = $fs[$row['id']];
        } else {
          $values = array($row['id'] => $fs[$row['id']]);
          $authinfo = $fs;
        }

        $values = wisski_pathbuilder_filter_auth_data($values);
        

        if(is_array($ov[$row['id']]))
          $old_values = $ov[$row['id']];
        else
          $old_values = array($row['id'] => $ov[$row['id']]);

//      list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($v, $value, $fs_pointer, $row['id']);        


//      drupal_set_message("My value: " . serialize($values));
//      drupal_set_message("My auth: " . serialize($authinfo));

      // Delete the old ones
        foreach($old_values as $key => $value) {
//          drupal_set_message("delete old ones!");
          if($value == $values[$key] || empty($value))
            continue;
//          drupal_set_message("really delete: " . $value);
//        drupal_set_message("value is: " . $value);
//        drupal_set_message("ov is: ". serialize($old_values[$key])); 
        
//        $spq = wisski_pathbuilder_get_sparql($row['id'], $condition);
      
//        $sqrows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($spq, 'rows');
      
        
//        foreach($sqrows as $one_row) {

          $sparql = wisski_pathbuilder_get_sparql($row['id'], $condition, TRUE, 0, FALSE);
          $data = wisski_pathbuilder_get_group_samepart_data($groupid);
//          drupal_set_message("data is: " . serialize($data));
//          drupal_set_message("groupid : " . serialize($groupid));
      
          if($row['disamb'] > 0) 
            $delete = ("DELETE { " . $sparql['triples'][1+(($row['disamb'] - 2) * 2) ] . " . } WHERE { ");
          else
            $delete = ("DELETE { " . $sparql['triples'][count($data)] . " . } WHERE { ");
                      
          foreach($sparql['triples'] as $triple) {
            $delete .= ($triple .  " . ");
          }

          $delete .= $condition . '. FILTER ( ?out = "' . wisski_store_escape_sparql_literal($value) . '" ) }';
 //         drupal_set_message("I am deleting: " . htmlentities($delete));

          wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($delete);
//        }
        }
        
//        drupal_set_message(serialize($values));
        // add the new ones
        
        foreach($values as $key => $value) {
//          drupal_set_message(serialize($values));
//          drupal_set_message("key ist: $key value ist: " . $value);	
          if(empty($value) || $value == "" || $old_values[$key] == $value)
            continue;
//          drupal_set_message("paths sind: " . serialize($paths));
//          drupal_set_message("v ist: " . $v);
          if($value) {
//            drupal_set_message(serialize($values));
//            drupal_set_message("key ist: $key value ist: " . $value);
                                
            //preg_match('/\d+$/', $key, $matches);
            //$auth_key = $matches[0];
            
            if(strpos($key, '_') !== FALSE)
              $auth_key = str_replace('_', '_auth_data_', $key);
            else
              $auth_key = 'auth_data_' . $key; 

//            drupal_set_message("auth key ist: " . $auth_key);
//            drupal_set_message("fs is: " . serialize($fs));
//            list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($auth_key, $value, $authinfo);
//            drupal_set_message("indiv: " . $indiv . " triples: " . serialize($auth_triples) . " x " . $x);

//            drupal_set_message("paths sind: " . serialize($paths));
//            drupal_set_message(" v is: " . $v);
//            drupal_set_message("mein path ist: " . serialize($paths[$v]));
//            drupal_set_message("MS ist: " . $main_subject);


          
            // d.h. die Gruppe gabs schon
            if(!empty($paths[$v]['path'])) {
//              drupal_set_message("There is a path");
              $data = $paths[$v]['path'];
   //           drupal_set_message(serialize($data));
              $sp = _wisski_pathbuilder_calculate_group_samepart($groupid);
   //           drupal_set_message(serialize($sp));

              list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($auth_key, $value, $authinfo);
              if (empty($indiv)) {
                $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($sp)/2)], $data);
              } else {
                $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($sp)/2)], $data, $indiv, $x);
                $triples = array_merge($triples, $auth_triples);
              }
              
            
            } // die gruppe gabs noch nicht 
            else if(empty($paths[$v]['path'])) {
              // wenn das parent 0 ist, ist es eine Hauptgruppe -> einfach generieren
              if(!$my_group['group_id']) {
//                drupal_set_message("it is a main group");
//                drupal_set_message("MS ist: " . $main_subject);
                if(!empty($main_subject)) {
//                  drupal_set_message("There is a ms!", 'error');
                  $data = wisski_pathbuilder_get_group_samepart_data($my_group['id'], $main_subject);
//                  $sp = _wisski_pathbuilder_calculate_group_samepart($groupid);
                  
//                  drupal_set_message("data is: " . serialize($data));
                  
                  list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($auth_key, $value, $authinfo);
                  if (empty($indiv)) {
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data);
                  } else {
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data, $indiv, $x);
                    $triples = array_merge($triples, $auth_triples);
                  }
//                  $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data);
                } else {
//                  drupal_set_message("There is no MS!");
                  list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($auth_key, $value, $authinfo);
                  if (empty($indiv)) {
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id']);
                  } else {
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], '', array(), $indiv, $x);
                    $triples = array_merge($triples, $auth_triples);
                  }
//                  $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id']);
                  $main_subject = $triples[0]['s'];
                  $condition = "FILTER ( ?x0 = <" . $main_subject . ">) ";
                }
              } else {
//               drupal_set_message("it is no main group");
                if(!empty($main_subject)) {
//                  drupal_set_message("there is a ms!");
                  //$data = wisski_pathbuilder_get_group_samepart_data($my_group['id'], $main_subject);
                  //drupal_set_message("data is: " . serialize($data));
                  //if(empty($data)) {
                  
//                  drupal_set_message("parent is: " . $parentkey . " and has value: " . serialize($paths[$parentkey]));
                  
                  if($parentpath)
                    $data = $parentpath;
                  else
                    $data = wisski_pathbuilder_get_group_samepart_data($my_group['group_id'], $main_subject);
//                  drupal_set_message("my data is: " . serialize($data));
//                  drupal_set_message("I am generating triples for " . $row['id']);
                  //}
                  list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($auth_key, $value, $authinfo);

//                  drupal_set_message("indiv: " . $indiv);
//                  drupal_set_message(" triples: " . serialize($auth_triples));
//                  drupal_set_message(" x " . $x);

                  if (empty($indiv)) {
//                    drupal_set_message("no individual!");
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data);
                  } else {
                    
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data, $indiv, $x);
                    $triples = array_merge($triples, $auth_triples);
                  }
//                  $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data);
//                  drupal_set_message("I made triples: " . serialize($triples));
                  
                  /*
                  $sp = _wisski_pathbuilder_calculate_group_samepart($groupid);
                  $path = array();
                  drupal_set_message("samepart is: " . serialize($sp));
                  drupal_set_message("triples: " . serialize($triples));
                  $path['x0'] = $main_subject;
                  for($i=0; $i<ceil(count($sp)/2); $i++) {
                    $class = $sp['x' . $i];
                    foreach($triples as $triple) {
                      drupal_set_message("comparing: " . $triple['p'] . " to " . wisski_store_getObj()->wisski_ARCAdapter_delNamespace('rdf:type'));
                      drupal_set_message("and then comparing: " . $triple['o'] . " to " . $class);
                      if($triple['p'] == wisski_store_getObj()->wisski_ARCAdapter_delNamespace('rdf:type') && $triple['o'] == $class) {
                        drupal_set_message("success! " . $i . " is now " . $triple['s']);
                        $path['x' . $i] = $triple['s'];
                        continue;
                      }
                    }
                    if($i < floor(count($sp)/2))
                      $path['y' . $i] = $sp['y' . $i];
                  }
                  */
                  
                  $path = $data;
 //                 drupal_set_message("got data: " . serialize($data));
                  $sp = _wisski_pathbuilder_calculate_group_samepart($groupid);
                  
                  $m_triples = $triples;
                  
                  for($i=floor(count($data)/2); $i<floor(count($sp)/2); $i++) {
                    $pred = $sp['y' . $i];
                    $path['y' . $i] = $pred;
                    
                    foreach($m_triples as $m_key => $triple) {
                      if($triple['p'] == $pred) {
                        $path['x' . ($i+1)] = $triple['o'];
                        unset($m_triples[$m_key]);
                        continue;
                      }
                    }
                  }
                  
                  $paths[$v]['path'] = $path;
//                  drupal_set_message("the paths are: " . serialize($paths));
//                  $paths[$v] = wisski_pathbuilder_get_group_samepart_data($my_group['id'], $main_subject);
                  //drupal_set_message("samepart ist: " . wisski_pathbuilder_get_group_samepart_data($my_group['id'], $main_subject));
                } else {// else kann eigentlich nicht passieren, da obige rekursion ziehen hätte müssen
                  //$data = wisski_pathbuilder_get_group_samepart_data($my_group['group_id']);
//                  drupal_set_message("this should not happen!");
                  list($indiv, $auth_triples, $x) = wisski_pathbuilder_check_auth_triples($auth_key, $value, $authinfo);
                  if (empty($indiv)) {
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id']);
                  } else {
                    $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], '', array(), $indiv, $x);
                    $triples = array_merge($triples, $auth_triples);
                  }
//                  $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id']);
                  $main_subject = $triples[0]['s'];
                  $condition = "FILTER ( ?x0 = <" . $main_subject . ">) ";
                }
              }
            }
            
//            drupal_set_message("I used data: " . serialize($data));
//            drupal_set_message("I would generate: " . serialize($triples));
            wisski_store_getObj()->wisski_ARCAdapter_getStore()->insert($triples, wisski_store_getObj()->wisski_ARCAdapter_graphName(), FALSE);
            
            foreach($triples as $triple)
              if($triple['s'] != $main_subject)
                wisski_store_addNodeForTriple($triple);           
/*
            if(empty($paths[$v])) {
              $sparql = wisski_pathbuilder_get_sparql($row['id'], $condition . ' FILTER( ?out = "' . $value . '") ', TRUE, 0, FALSE);
              
              $pathres = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql, 'rows');
              $sp = _wisski_pathbuilder_calculate_group_samepart($groupid);
              
              $newpath = array();
              
              for($i=0; $i<floor(count($sp)/2); $i++) {
                $newpath['x' . $i] = $pathres[0]['x'. $i];
                $newpath['y' . $i] = $sp['y' . $i];
              }
              $newpath['x' . $i] = $pathres[0]['x'. $i];
              $paths[$v] = $newpath;
//              $paths[$v] = wisski_pathbuilder_get_group_samepart_data($my_group['id'], $main_subject);
            }
*/
            $got_triples = array_merge($got_triples, $triples);

//            if(!empty($old_values[$key]))
          // wenns schon da war -> Suchen
          // wenns noch nicht da war -> neu
          // wenns verändert worden ist -> 

/*
          if(isset(
          drupal_set_message("got a value: " . $value);
//            $data = wisski_pathbuilder_get_group_samepart_data($groupid);
            $pathdata = $paths[$groupid];
            if($my_group['fieldtype'] == "ontogroup") {
              $data = wisski_pathbuilder_get_group_samepart_data($groupid);
              $count = floor(count($data)/2);
              unset($data['x' . $count]);
              unset($data['y' . $count-1]);
              $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data);
            } else {
              $data = wisski_pathbuilder_get_group_samepart_data($groupid);
              $triples = _wisski_pathbuilder_generate_triples_of_path($form, $value, $row['id'], $data["x" . floor(count($data)/2)], $data);
            }
            wisski_store_getObj()->wisski_ARCAdapter_getStore()->insert($triples, wisski_store_getObj()->wisski_ARCAdapter_graphName(), FALSE);

            foreach($triples as $triple)
              wisski_store_addNodeForTriple($triple);
              */
          }
        }
        
      }  
    }
  }
  return $got_triples;
}


function _wisski_pathbuilder_generate_triples_of_path($form, $valuestr, $id, $subjIn = "", $samepart = array(), $objIn = NULL, $obj_x = -1) {
  if(empty($valuestr))
    return array();
  $result = db_query("SELECT * FROM {wisski_template} WHERE id = $id ");

  $sameMax = floor(count($samepart)/2);
  $outTriples = array();  

  $row = db_fetch_array($result);

  if($row['disamb'] > 1) {
    if (empty($objIn)) {
      $sparql = wisski_pathbuilder_get_sparql($id, 'FILTER ( ?out = "' . wisski_store_escape_sparql_literal($valuestr) . '" ) ', FALSE, $row['disamb']-1);
      $disambrows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql, 'rows');
      //drupal_set_message("disambrows: " . serialize($disambrows));
    } elseif ($obj_x == -1) {
      $disambrows = array('a'); // dummy binding so that we have disambx set correctly
    } else {
      $disambx = $obj_x +1; // ensure dismabx is set; set greater than $objx so that it doenst interfere in triple gen
    }

    if(count($disambrows) > 0)
      $disambx = $row['disamb'] -1;
  }

  $subj = $subjIn;
  
  if($disambx && $disambx > 0 && isset($disambx) && ($disambx -1) <= floor(count($samepart)/2) ) {
    $subj = $samepart["x" . ($disambx - 1) ];
  }
  
  $path = unserialize($row['path_array']);
  $tmpTriples = array();
  $i=0;

  
  // if subj is empty, we need to create 
  if($subj == "") {
      $subj = wisski_store_getObj()->wisski_ARCAdapter_delNamespace(wisski_store_makePathTitle(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($path["x$sameMax"])) . wisski_store_getObj()->wisski_Store_getNewUniqueID());
      $trip = array();
      $trip['s'] = $subj;
      $trip['s_type'] = "uri";
      $trip['p'] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type");
      $trip['o'] = $path["x$sameMax"];
      $trip['o_type'] = "uri";
      $tmpTriples[] = $trip;
  }


  for($i=$sameMax; $i < floor(count($path)/2); $i++) {


    if($obj_x - 1 == $i) {
        $obj = $objIn;
        $trip = array();
        $trip['s'] = $subj;
        $trip['s_type'] = "uri";
        $trip['p'] = $path["y" . ($i)];
        $trip['o'] = $obj;
        $trip['o_type'] = "uri";
        $tmpTriples[] = $trip;
        break; //return $tmpTriples;
    }

    if($disambx && isset($disambx)) {
      if(($disambx) == $i) {
        if (empty($objIn)) {
          $objIn = $disambrows[0]['x' . ($disambx)];
        }// elseif ($obj_x != -1) {
        $obj = $objIn;
        //}
        $trip = array();
        $trip['s'] = $subj;
        $trip['s_type'] = "uri";
        $trip['p'] = $path["y" . ($i -1)];
        $trip['o'] = $obj;
        $trip['o_type'] = "uri";
        $tmpTriples[] = $trip;
        break; //return $tmpTriples;
      }

      if(($disambx - 1) == $i) {
        if (empty($objIn)) {
          $objIn = $disambrows[0]['x' . ($disambx)];
        }// elseif ($obj_x != -1) {
        $obj = $objIn;
        //}
        $trip = array();
        $trip['s'] = $subj;
        $trip['s_type'] = "uri";
        $trip['p'] = $path["y" . ($i)];
        $trip['o'] = $obj;
        $trip['o_type'] = "uri";
        $tmpTriples[] = $trip;
        break; //return $tmpTriples;
      }

    }

    $obj = wisski_store_getObj()->wisski_ARCAdapter_delNamespace(wisski_store_makePathTitle(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($path["x". ( $i + 1 )])) . wisski_store_getObj()->wisski_Store_getNewUniqueID());
    
    $trip = array();
    $trip['s'] = $subj;
    $trip['s_type'] = "uri";
    $trip['p'] = $path["y$i"];
    $trip['o'] = $obj;
    $trip['o_type'] = "uri";
    $tmpTriples[] = $trip;

    $trip = array();
    $trip['s'] = $obj;
    $trip['s_type'] = "uri";
    $trip['p'] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type");
    $trip['o'] = $path[("x" . ( $i + 1 ))];
    $trip['o_type'] = "uri";
    $tmpTriples[] = $trip;

    $subj = $obj;
  } 

  $obj = $valuestr;

  if($obj != "" && isset($obj) && isset($disambx)) {
    $outTriples = array_merge($outTriples, $tmpTriples);
  }


  if(empty($objIn) || !$disambx || !isset($disambx)) {
    if(empty($row['datatype_property'])) {
      $datatypeprops = wisski_store_getObj()->wisski_ARCAdapter_getDatatypePropertiesForConcept($path[("x" . ($i))]);
      $datatypeprop = $datatypeprops[0];
    } else
      $datatypeprop = $row['datatype_property'];

    $range = wisski_store_getObj()->wisski_ARCAdapter_delNamespace("xsd:string");

    $q = "SELECT ?z WHERE { <$datatypeprop> <http://www.w3.org/2000/01/rdf-schema#range> ?z }";

    if($rows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q, 'rows')) {
      if(isset($rows[0]['z']))
        $range = $rows[0]['z'];
    }

    $trip = array();
    $trip['s'] = $subj;
    $trip['s_type'] = "uri";
    $trip['p'] = $datatypeprop; //wisski_store_getObj()->wisski_ARCAdapter_delNamespace("ecrm:has_PrimitiveString");
    $trip['o'] = $obj;
    $trip['o_type'] = "literal";
    $trip['o_datatype'] = $range; //wisski_store_getObj()->wisski_ARCAdapter_delNamespace("xsd:string");
    $tmpTriples[] = $trip;
    if($obj != "") 
      $outTriples = array_merge($outTriples, $tmpTriples);
  }        

  return $outTriples;
}

function wisski_pathbuilder_template_view($form_state, $node, &$groupid = -1, $samepart = array()) {
//  drupal_set_message(serialize($form_state));
//  drupal_set_message(serialize($node));
  
  if($groupid == -1) {
    $groupid = wisski_pathbuilder_getGroupIDForIndividual(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title));
  }

//  drupal_set_message("groupid: " . $groupid);

  $samepart = _wisski_pathbuilder_calculate_group_samepart($groupid);
  $sparqlcondition = (" FILTER ( ?x" . (floor(count($samepart)/2)) . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title). "> ) ");
  $paths = array();
  // $form = _wisski_pathbuilder_recursive_build_group(array(), $groupid, $paths, $input = 0, $sparqlcondition, 2);
  $form = _wisski_pathbuilder_recursive_build_group($form_state['values'], $groupid, $paths, $input = 0, $sparqlcondition, 2);

  return $form;  

}

function wisski_pathbuilder_path_view($form_state, $node) {
  $groupid = wisski_pathbuilder_getGroupIDForIndividual(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title));

  $samepart = _wisski_pathbuilder_calculate_group_samepart($groupid);
  $sparqlcondition = (" FILTER ( ?x" . (floor(count($samepart)/2)) . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title). "> ) ");
  $form = _wisski_pathbuilder_recursive_build_group($form_state['values'], $groupid, array(), -1, $sparqlcondition);

  return $form;

}


  /**
   * Form for editing an entire menu tree at once.
   *
   * Shows for one menu the menu items accessible to the current user and
   * relevant operations.
   */

  function wisski_pathbuilder_overview_form(&$form_state, $menu, $type = 2) {

    global $menu_admin;

    $form = _wisski_pathbuilder_make_form($type);



    $form['#menu'] =  $menu;

    if (element_children($form)) {
      $form['markup'] = array(
          '#prefix' => '<div class="wki-admin-buttons">',
          '#suffix' => '</div>',
          );

      $form['markup']['submit'] = array(
          '#type' => 'submit',
          '#value' => t('Save configuration'),
          );

      $form['markup']['add_group'] = array(
          '#type' => 'submit',
          '#value' => t('Add Group'),
          '#submit' => array('wisski_pathbuilder_add_group'),
          );

      $form['markup']['add_path'] = array(
          '#type' => 'submit',
          '#value' => t('Add Path'),
          '#submit' => array('wisski_pathbuilder_add_own_path'),
          );

    }
    else {
      $form['empty_menu'] = array('#value' => t('There are no items yet. Please add some.'));
    }
    
    $form['export'] = array(
      '#collapsed' => TRUE,
      '#type' => 'fieldset',
      '#collapsible' => '1',
      '#title' => t('Export Paths'),
    );

    $form['export']['exp_markup'] = array(
      '#value' => '<strong>' . t('Currently available dumps:') . '</strong><br/>',
      '#prefix' => '<div class="wki-inputbox">',
      '#suffix' => '</div>',
    );

    $curExpos = variable_get("wisski_pb_export", array());
    $newExpos = array();
    $delbuttons = array();
    $str = "";
    if(!empty($curExpos)) {
      
      $form['export']['exp_markup']['table'] = array(
        '#type' => 'form',
        '#suffix' => '</table>',
        '#prefix' => '<table><tr><th>' . t('filename') . '</th><th>' . t('filesize') . '</th><th>' . t('date') . '</th><th>' . t('operations') . '</th></tr>',
      );
      
      foreach($curExpos as $curExpo) {
        if(file_exists($curExpo['file'])) {
          $form['export']['exp_markup']['table'][$curExpo['file']] = array(
            '#type' => 'form',
            '#prefix' => '<tr>',
            '#suffix' => '</tr>',
          );
          
          $newExpos[] = $curExpo; 
      
          $str = '<td><a href="' . file_create_url($curExpo['file']) . '">' . 
            basename($curExpo['file']) . '</a></td><td>' . $curExpo['size'] . 
            ' Bytes</td><td>' . $curExpo['date'] . '</td>';
          
          $form['export']['exp_markup']['table'][$curExpo['file']]['str'] = array(
            '#value' => $str,
          );
          
          $form['export']['exp_markup']['table'][$curExpo['file']]['wisski_pathbuilder_delExpo_' . basename($curExpo['file'])] = array(
            '#type' => 'submit',
            '#name' => basename($curExpo['file']),
            '#prefix' => '<td>',
            '#suffix' => '</td>',
            '#value' => t('Delete Export ') . basename($curExpo['file']),
            '#submit' => array('wisski_pathbuilder_delExpo'),
            '#attributes' => array('class' => 'wki-delete'),
        	);
        	$str = "";
        }
      }
    } else {
      $str = "None";
    }

    variable_set("wisski_pb_export", $newExpos);

    if(!empty($str));
      $form['export']['exp_markup']['#value'] .= $str;
    
    $form['export']['exp_markup']['export'] = array(
      '#type' => 'submit',
      '#value' => t('Create new export file'),
      '#submit' => array('wisski_pathbuilder_export_paths'),
    );
    
    $form['import'] = array(
      '#collapsed' => TRUE,
      '#type' => 'fieldset',
      '#collapsible' => '1',
      '#title' => t('Import Paths'),
    );
    
    $form['import']['imp_markup'] = array(
      '#prefix' => '<div class="wki-inputbox">',
      '#suffix' => '</div>',
    );

    $form['import']['imp_markup']['import_pb'] = array(
      '#type' => 'textfield',
      '#title' => t('Load Pathbuilder-Definitions'),
      '#default_value' => "", 
      '#description' => t("The url of the file that should be loaded in WissKI."),
    );
    
    $form['import']['imp_markup']['import'] = array(
      '#type' => 'submit',
      '#value' => t('Load import file'),
      '#submit' => array('wisski_pathbuilder_import_paths'),
    );

    return $form;

  }

  /**
  * Deletion of a dumpfile 
  * @author: Mark Fichtner
  * @return: TRUE if the dumpfile was deleted
  */
  function wisski_pathbuilder_delExpo($f, $form_state) {
    drupal_set_message("Deleting " . file_directory_path() . '/' . $form_state['clicked_button']['#name']);
    file_delete(file_directory_path() . '/' . $form_state['clicked_button']['#name']);
    return TRUE;
  }

  function wisski_pathbuilder_export_paths() {
    $tree = arg(4);
    
    if(empty($tree))
      $tree = 2;

    $xmldoc = new SimpleXMLElement("<pathbuilderinterface></pathbuilderinterface>");

    wisski_pathbuilder_export_group(0, $tree, $xmldoc);

    $dom = dom_import_simplexml($xmldoc)->ownerDocument;
    $dom->formatOutput = true;
    
    $file = file_save_data($dom->saveXML(), $file);

    $cur = variable_get("wisski_pb_export", array());
    $cur[] = array('file' => $file, 'size' => filesize($file), 'date' => date('Y-m-d H:i:s'));
    variable_set("wisski_pb_export", $cur);
  }
  
  function wisski_pathbuilder_export_group($groupid, $tree, $xmldoc) {
    $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = " . $groupid);
    
    while($arr = db_fetch_array($result)) {
    
      $this_path = $xmldoc->addChild("path");
// changes by Martin Scholz
// the former code produces crappy xml as entities get not escaped properly
// old      foreach($arr as $key => $value)
//        $this_path->addChild($key), htmlentities($value));
      foreach($arr as $key => $value) {
        if ($key == 'path_array') {
          $value = unserialize($value);
          $pa_node = $this_path->addChild($key);
          foreach ($value as $k => $v) {
            $pa_node->addChild($k, htmlspecialchars($v));
          }
        } else {
          $this_path->addChild($key, htmlspecialchars($value));
        }
      }
// end changes
      if($arr['is_group'])
        wisski_pathbuilder_export_group($arr['id'], $tree, $xmldoc);
    }
    
  }
  
  function wisski_pathbuilder_import_paths($form, &$form_state) {
    $importfile = $form_state['values']['import']['imp_markup']['import_pb'];

    $xmldoc = new SimpleXMLElement($importfile, 0, TRUE);
    
    foreach($xmldoc->path as $path) {
    
      $parentid = html_entity_decode((int)$path->group_id);
      
      if($parentid != 0) 
        $parentid = wisski_pathbuilder_check_parent($parentid, $xmldoc);
      
      $uuid = html_entity_decode((string)$path->uuid);
      if(empty($uuid))
        $path->uuid = wisski_get_uid();
    
      $result = db_query("SELECT * FROM {wisski_template} WHERE uuid = '" . html_entity_decode((string)$path->uuid) . "'");
      
// inserted by martin scholz
// see ..._export_paths()
      $path_array = array();
      foreach ($path->path_array->children() as $n) {
        $path_array[$n->getName()] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace((string) $n);
      }
      $path_array = serialize($path_array);
// end insert
      
      if($arr = db_fetch_array($result)) { // update
        db_query("UPDATE {wisski_template} SET 
          path_array = '%s', datatype_property = '%s', short_name = '%s', disamb = '%s', 
          length = %d, name = '%s', description = '%s', 
          group_id = %d, weight = %d, is_group = %d, fieldtype = '%s', enabled = %d, 
          mandatory = %d, repeatable = %d, type = %d WHERE uuid = '%s'", 
          $path_array,
// by Martin Scholz           html_entity_decode((string)$path->path_array),
          wisski_store_getObj()->wisski_ARCAdapter_delNamespace(html_entity_decode((string)$path->datatype_property)),
          html_entity_decode((string)$path->short_name),
          html_entity_decode((string)$path->disamb), 
          html_entity_decode((int)$path->length), 
          html_entity_decode((string)$path->name), 
          html_entity_decode((string)$path->description), 
          $parentid, 
          html_entity_decode((int)$path->weight), 
          html_entity_decode((int)$path->is_group), 
          html_entity_decode((string)$path->fieldtype), 
          html_entity_decode((int)$path->enabled), 
          html_entity_decode((int)$path->mandatory), 
          html_entity_decode((int)$path->repeatable), 
          html_entity_decode((int)$path->type),
          html_entity_decode((string)$path->uuid));
      } else { // insert
        db_query("INSERT INTO {wisski_template} SET 
          uuid = '%s', path_array = '%s', datatype_property = '%s', short_name = '%s', disamb = '%s', 
          length = %d, name = '%s', description = '%s', 
          group_id = %d, weight = %d, is_group = %d, fieldtype = '%s', enabled = %d, 
          mandatory = %d, repeatable = %d, type = %d", 
          html_entity_decode((string)$path->uuid),
          $path_array,
// by Martin Scholz           html_entity_decode((string)$path->path_array),
          wisski_store_getObj()->wisski_ARCAdapter_delNamespace(html_entity_decode((string)$path->datatype_property)),
          html_entity_decode((string)$path->short_name),
          html_entity_decode((string)$path->disamb), 
          html_entity_decode((int)$path->length), 
          html_entity_decode((string)$path->name), 
          html_entity_decode((string)$path->description), 
          $parentid, 
          html_entity_decode((int)$path->weight), 
          html_entity_decode((int)$path->is_group), 
          html_entity_decode((string)$path->fieldtype), 
          html_entity_decode((int)$path->enabled), 
          html_entity_decode((int)$path->mandatory), 
          html_entity_decode((int)$path->repeatable), 
          html_entity_decode((int)$path->type));
      }
      
    }
    
  }
  
  function wisski_pathbuilder_check_parent($parentid, $xmldoc) {
    if($parentid == 0) 
      return 0;
      
    foreach($xmldoc->path as $path) {
      if($parentid == html_entity_decode((int)$path->id)) {
        $parentuuid = html_entity_decode((string)$path->uuid);
        
        $result = db_query("SELECT * FROM {wisski_template} WHERE uuid = '" . $parentuuid . "'");
        
        if($arr = db_fetch_array($result)) {
          return $arr['id'];
        } else {
         $myparentid = wisski_pathbuilder_check_parent(html_entity_decode((int)$path->group_id), $xmldoc);
         
// inserted by martin scholz
// see ..._export_paths()
          $path_array = array();
          foreach ($path->path_array->children() as $n) {
            $path_array[$n->getName()] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace((string) $n);
          }
          $path_array = serialize($path_array);
// end insert

          db_query("INSERT INTO {wisski_template} SET 
          uuid = '%s', path_array = '%s', datatype_property = '%s', short_name = '%s', disamb = '%s', 
          length = %d, name = '%s', description = '%s', 
          group_id = %d, weight = %d, is_group = %d, fieldtype = '%s', enabled = %d, 
          mandatory = %d, repeatable = %d, type = %d", 
          html_entity_decode((string)$path->uuid),
          $path_array,
// by Martin Scholz          html_entity_decode((string)$path->path_array),
          wisski_store_getObj()->wisski_ARCAdapter_delNamespace(html_entity_decode((string)$path->datatype_property)),
          html_entity_decode((string)$path->short_name),
          html_entity_decode((string)$path->disamb), 
          html_entity_decode((int)$path->length), 
          html_entity_decode((string)$path->name), 
          html_entity_decode((string)$path->description), 
          $parentid, 
          html_entity_decode((int)$path->weight), 
          html_entity_decode((int)$path->is_group), 
          html_entity_decode((string)$path->fieldtype), 
          html_entity_decode((int)$path->enabled), 
          html_entity_decode((int)$path->mandatory), 
          html_entity_decode((int)$path->repeatable), 
          html_entity_decode((int)$path->type));
          
          $result = db_query("SELECT * FROM {wisski_template} WHERE uuid = '" . $parentuuid . "'");
          $arr = db_fetch_array($result);
          
          return $arr['id'];
        }
      }
    }
  }
  
  
  
  function _wisski_pathbuilder_return_item() {
    $item = array();
    $item["#item"] = array();
    $item["#item"]["title"] = "";
    $item["#item"]["description"] = "";
    $item["#item"]["mlid"] = 0;
    $item["#item"]["plid"] = 0;
    $item["#item"]["options"] = array();
    $item["#item"]["hidden"] = 0;
    $item["#item"]["has_children"] = 0;
    $item["#item"]["weight"] = -10; 
    $item["#item"]["depth"] = 1;
    $item["#item"]["href"] = "";
    $item["#attributes"]["class"] = "wisski-enabled";
    $item["title"]["#value"] = "";
    $item["enabled"]["#type"] = "checkbox";
    $item["enabled"]["#default_value"] = 0;
    $item["weight"]["#type"] = "weight";
    $item["weight"]["#delta"] = 50;
    $item["weight"]["#default_value"] = 0;
    $item["mlid"]["#type"] = "hidden";
    $item["mlid"]["#value"] = 0;
    $item["plid"]["#type"] = "textfield";
    $item["plid"]["#default_value"] = 0;
    $item["plid"]["#size"] = 6;
    $item["operations"] = array();                       

    return $item;                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                
  }

/**
 * Generates the sparql-query for the according group or path. If a group is
 * given the primitive data value is omitted. If a special individual should
 * be used as starting point for the path, it can be given in the conditions
 * parameter. Typical style is "FILTER ( ?x0 = <http://examp.le/thing> )".
 * All filters of this style are replaced by direct querys and the parameter
 * is not returned. Don't rely on the returning! 
 * This function is sensitive to the variable wisskiReasoner_inverse_reasoning.
 * @param id the id of a path or a group. if the id is invalid, the empty string will be returned.
 * @param conditions additional conditions which are appended at the end of the query string
 * @param triplewise usually false to get normal query-able string by default, can be used to get triple-representation
 * @param startpoint sets the starting point of the path. Usually 0 for the beginning concept, 1 the next, etc.
 * @return A string suitable for sparql-querying if not triplewise, else an array.
 * @author Mark Fichtner
 */
function wisski_pathbuilder_get_sparql($id, $conditions = "", $triplewise = FALSE, $startpoint = 0, $inverse = TRUE, $returnvars = array()) {
  $result = db_query("SELECT * FROM {wisski_template} WHERE id = " . $id);
  $item = db_fetch_array($result);

  if(!$item) {
    drupal_set_message("WisskiTemplate: Could not resolve path or group with id $id - there is no such id.","error");
    return "";
  }

  if($item['is_group']) 
    $path = _wisski_pathbuilder_calculate_group_samepart($id);
  else
    $path = unserialize($item["path_array"]);

  if($item['is_group'])
    $item['length'] = floor(count($path)/2);

  if(empty($item['datatype_property'])) {
    $datatypeprops = wisski_store_getObj()->wisski_ARCAdapter_getDatatypePropertiesForConcept($path[("x" . $item["length"])]);
    $datatypeprop = $datatypeprops[0];
      
    if(empty($datatypeprop)) {
      $datatypeprop = "none possible.";
      return "";
    }
        
    db_query("UPDATE {wisski_template} SET datatype_property = '" . $datatypeprop . "' WHERE id = " . $id);
  } else
    $datatypeprop = $item['datatype_property'];

  if(!$triplewise) {
    $sparql = "";

    if(!variable_get("wisskiReasoner_inverse_reasoning",FALSE) || !$inverse) {
      $sparqlstart = "SELECT ";
    } else {
      $sparqlstart = "SELECT DISTINCT ";
    }
    $sparqlmid = "WHERE { ";

    // if the user told us what variables to select, take these!
    // e.g. "only select out"
    if(!empty($returnvars))
      foreach($returnvars as $returnvar)
        $sparqlstart .= $returnvar . ' ';

    // if inverse reasoning is enabled, do it!
    if(!variable_get("wisskiReasoner_inverse_reasoning",FALSE) || !$inverse) {
      for($ii=$startpoint; $ii<floor(count($path)/2); $ii++) {
        // if the user told us what he wants, fetch this otherwise select everything
        if(empty($returnvars))
          $sparqlstart .= "?x$ii ";
        $sparqlmid .= "?x$ii rdf:type <" . $path["x$ii"] . "> . ?x$ii <" . $path["y$ii"] . "> ?x" . ($ii + 1) . " . ";
      }
    } else {
      for($ii=$startpoint; $ii<floor(count($path)/2); $ii++) {
        // if the user told us what he wants, fetch this otherwise select everything
        if(empty($returnvars))
          $sparqlstart .= "?x$ii ";
        // we have to check whether property really has an inverse!
        $inverse = wisski_reasoner_getInverse($path["y$ii"]);
        if (!$inverse) {
          $sparqlmid .= "?x$ii rdf:type <" . $path["x$ii"] . "> . ?x$ii <" . $path["y$ii"] . "> ?x" . ($ii + 1) . " . ";
        } else {
          $sparqlmid .= "?x$ii rdf:type <" . $path["x$ii"] . "> . { { ?x$ii <" . $path["y$ii"] . "> ?x" . ($ii + 1) . " } " .
            "UNION { ?x" . ($ii + 1) . " <" . $inverse . "> ?x$ii } } . ";
        }
      } 
    }

    // if this is the sparql-query for a group, don't append any primitive data properties      
    if($item['is_group']) {
      if(empty($returnvars))
        $sparqlstart = $sparqlstart . "?x" . floor(count($path)/2) . " ";
      $sparqlmid = $sparqlmid . "?x" . floor(count($path)/2) . " rdf:type <". $path[("x" . floor(count($path)/2))] . "> . ";

      $sparql = $sparqlstart . $sparqlmid . $conditions . " }";
    // if it is a path, append the primitive data property
    } else {
      if(empty($returnvars))
        $sparqlstart = $sparqlstart . "?x" . floor(count($path)/2) . " ?out ";
      $sparqlmid = $sparqlmid . "?x" . floor(count($path)/2) . " rdf:type <". $path[("x" . floor(count($path)/2))] . "> . ?x" . 
        floor(count($path)/2) . " <" . $datatypeprop . "> ?out . ";

      $sparql = $sparqlstart . $sparqlmid . $conditions . " }";
    }      

    return $sparql;
  }

  if($triplewise) {
    $sparql = array();

    if(!variable_get("wisskiReasoner_inverse_reasoning",FALSE) || !$inverse)
      $sparqlstart = "SELECT ";
    else
      $sparqlstart = "SELECT DISTINCT ";
      
    $sparql['mid'] = "WHERE { ";
    $sparqltriples = array();
    $elements = array();
    
    // if the user told us what variables to select, take these!
    // e.g. "only select out"
    if(!empty($returnvars))
      foreach($returnvars as $returnvar) {
        $sparqlstart .= $returnvar . ' ';
        $elements[] = $returnvar;
      }

    if(!variable_get("wisskiReasoner_inverse_reasoning",FALSE) || !$inverse) {
      for($ii=$startpoint; $ii<floor(count($path)/2); $ii++) {
        if(empty($returnvars)) {
          $sparqlstart .= "?x$ii ";
          $elements[] = "?x$ii";
        }
        
        $sparqltriples[] = "?x$ii rdf:type <" . $path["x$ii"] . "> ";
        $sparqltriples[] = "?x$ii <" . $path["y$ii"] . "> ?x" . ($ii + 1) . "";
      }
    } else {
      for($ii=$startpoint; $ii<floor(count($path)/2); $ii++) {
        if(empty($returnvars)) {
          $sparqlstart .= "?x$ii ";
          $elements[] = "?x$ii";
        }
        
        $sparqltriples[] = "?x$ii rdf:type <" . $path["x$ii"] . "> ";
        // we have to check whether property really has an inverse!
        $inverse = wisski_reasoner_getInverse($path["y$ii"]);
        if (!$inverse) {
          $sparqltriples[] = "?x$ii <" . $path["y$ii"] . "> ?x" . ($ii + 1) . "";
        } else {
          $sparqltriples[] = "{ { ?x$ii <" . $path["y$ii"] . "> ?x" . ($ii + 1) . 
            " } UNION { ?x" . ($ii + 1) . " <" . $inverse . 
            "> ?x$ii } } ";
        }
      }
    }

    $elements[] = ("?x" . floor(count($path)/2));
    if(!$item['is_group']) {
      if(empty($returnvars)) {
        $elements[] = "?out";
        $sparqlstart = $sparqlstart . "?x" . floor(count($path)/2) . " ?out";
      }
      $sparql["start"] = $sparqlstart;
    } else {
      if(empty($returnvars))
        $sparqlstart = $sparqlstart . "?x" . floor(count($path)/2);
      $sparql["start"] = $sparqlstart;
    }
    $sparql["end"] = $conditions . " }";

    $sparqltriples[] = ("?x" . floor(count($path)/2) . " rdf:type <" . $path[("x" . floor(count($path)/2))] . ">");

    if(!$item['is_group']) {
      if(empty($returnvars))
        $sparqltriples[] = ("?x" . floor(count($path)/2) . " <". $datatypeprop . "> ?out");
    
    }

    $sparql["triples"] = $sparqltriples;  
    $sparql["elements"] = $elements;
    $sparql["primitive"] = ("?x" . floor(count($path)/2) . " <". $datatypeprop . "> ?out");  
    $sparql["primitive_prop"] = $datatypeprop;
  }

  return $sparql;
}


  function _wisski_pathbuilder_make_form($type = 1) {
    static $form = array('#tree' => TRUE);

    $arr = array();

    $item = _wisski_pathbuilder_return_item();
    $item["#item"]["mlid"] = 0;
    $item["mlid"]["#value"] = 0;
    $item["#item"]["p1"] = 0;

    $item["#item"]["title"] = t("Ungrouped");
    $item["#item"]["href"] = "Ungrouped";
    $item["title"]["#value"] = t("Ungrouped");

    $item["#item"]["weight"] = -50;

    $item["#item"]["enabled"] = 1;
    $item["enabled"]["#default_value"] = 1;
    $item['weight'] = array(
        '#type' => 'weight',
        '#delta' => 50,
        '#default_value' => 0,
        );

    $item['plid'] = array(
        '#type' => 'textfield',
        '#default_value' => 0,
        '#size' => 6,
        );                      

    $form['wisski_pathbuilder_show'][0] = $item; 



    _wisski_pathbuilder_make_recursive_form($form, 0, $type);

    return $form;
  }


  function _wisski_pathbuilder_make_recursive_form(&$form, $thread_id, $type = 1) {

    $result = db_query("SELECT DISTINCT * FROM {wisski_template} WHERE group_id = " . $thread_id . " ORDER BY weight ASC");//ORDER BY concept_in ASC');

    while($concept = db_fetch_array($result)) {
      if(($concept['type'] % $type))
        continue;
      $item = _wisski_pathbuilder_return_item();
      $item["#item"]["mlid"] = $concept["id"];
      $item["mlid"]["#value"] = $concept["id"];
      $item["#item"]["p1"] = $concept["id"];

      $item["#item"]["title"] = t("Group");
      $item["#item"]["href"] = t("Group");

      $item["#item"]["depth"] = ($form['wisski_pathbuilder_show'][$concept["group_id"]]['#item']["depth"] + 1);

      if($concept["group_id"] == 0) {
        if($concept["is_group"] == 1) {
          $item["#item"]["depth"] = 1;
        } else {
          $item["#item"]["depth"] = 2;
        }
      }

      if($concept["is_group"] == 1) {
        $grouppath = _wisski_pathbuilder_calculate_group_samepart($concept["id"]);

        $ab = 0;
        $titel = t("Group ") . "["; 
        if(!empty($grouppath))
        foreach($grouppath as $conorprop) {
          if($ab > 0) 
            $titel .= " -> ";
          $ab++;
          $titel .= wisski_store_getObj()->wisski_ARCAdapter_addNamespace($conorprop);
        }
        $titel .= "]";
      } else {
        $item["plid"]["#default_value"] = $concept["group_id"];

        $titel = "";
        $ab = 0;

        foreach(unserialize($concept["path_array"]) as $conorprop) {
          if($ab > 0) 
            $titel .= " -> ";
          $ab++;
          $titel .= wisski_store_getObj()->wisski_ARCAdapter_addNamespace($conorprop);
        }
      }    


      $item["title"]["#value"] = $concept['name'];
      $item["#item"]["weight"] = $concept["weight"];

      $item["#item"]["enabled"] = $concept["enabled"];
      $item["enabled"]["#default_value"] = $concept["enabled"];
      $item['weight'] = array(
          '#type' => 'weight',
          '#delta' => 50,
          '#default_value' => $concept['weight'], 
          );

      $item['plid'] = array(
          '#type' => 'textfield',
          '#default_value' => $concept['group_id'],
          '#size' => 6,
          );

      $item['name']['#value'] = $titel; // $concept['name'];


      $operations = array();
      $operations['edit'] = l(t('edit'), 'admin/settings/wisski/pathbuilder/'. $item["#item"]["mlid"] .'/edit');
      $operations['delete'] = l(t('delete'), 'admin/settings/wisski/pathbuilder/'. $item["#item"]["mlid"] .'/delete');

      $item['operations'] = array();
      foreach ($operations as $op => $value) {
        $item['operations'][$op] = array('#value' => $value);
      }

      $form['wisski_pathbuilder_show'][$concept["id"]] = $item;

      _wisski_pathbuilder_make_recursive_form($form, $concept["id"], $type );
    }
  }



function wisski_pathbuilder_build_group($form_state, $parentID, $treetype = 2) {
  global $base_path;
  module_load_include('inc', 'node', 'node.pages');
  
  //$node = new stdclass(); 
  //$tmp = wisski_form($node, $form_state);

  
  //$tmp = module_invoke_all('form', $form_state);

  //drupal_set_message(serialize("I'm here!"));

/*
  unset($tmp['buttons']);
  unset($tmp['#validate']);
  unset($tmp['#theme']);
  unset($tmp['#id']);
  unset($tmp['nid']);
  unset($tmp['vid']);
  unset($tmp['uid']);
  unset($tmp['type']);
  unset($tmp['title']);
  unset($tmp['#node']);
*/
  $paths = array();
  
  if(!empty($form_state['values']))
    $form_state = $form_state['values'];
    
  $form = _wisski_pathbuilder_recursive_build_group($form_state, $parentID, $paths, 1, "", $treetype); 
//  $form = array_merge($form, $tmp);

  $form['group'] = array(
      '#type' => 'hidden',
      '#value' => $parentID,
      );

//      drupal_set_message(serialize($form['buttons']['submit']));
      
//  $form['buttons']['submit']['#submit'] = array_merge($form['buttons']['submit']['#submit'], array("wisski_pathbuilder_build_group_submit"));
  
//  drupal_set_message(serialize($form['buttons']['submit']));

/*
  $form['submit'] = array(
      '#type' => 'submit',
      '#value' => t('Save'),
      '#weight' => 60,
      );
*/

//  drupal_set_message(serialize(drupal_get_form('wisski_form')));
/*
global $user;

  $node = new stdclass();
  
  $node->user = $user->uid;
  $node->name = (isset($user->name) ? $user->name : '');
  $node->type = 'individual';
  $node->language = LANGUAGE_NONE;
*/
//  node_object_prepare($node);

//  return drupal_get_form('individual_node_form', $node);
//  return drupal_render($form);
//  drupal_set_message("I made out of it: " . serialize($form));
  return $form;

}

function wisski_pathbuilder_build_group_submit($form, &$form_state) {

  $groupid = $form_state['values']['group'];

/*
  $triples = _wisski_pathbuilder_gen_triples_of_group($form, $form_state, $groupid);

  $triple = $triples[0];

  $mainsubject = $triple['s'];

//  $form_state['redirect'] = $triple['s'];

//  drupal_set_message(serialize($triples));

//  drupal_set_message(serialize($form_state));

  wisski_store_getObj()->wisski_ARCAdapter_getStore()->insert($triples, wisski_store_getObj()->wisski_ARCAdapter_graphName(), FALSE);

  foreach($triples as $triple) {
    if($triple['s'] != $mainsubject)
      wisski_store_addNodeForTriple($triple);
  }
*/

//  drupal_set_message(serialize($form_state));

  $paths = array();


  $triples = wisski_pathbuilder_recursive_edit_submit($form, $form_state['values'], $groupid, $paths, array(), array());

  $triple = $triples[0];
  $mainsubject = $triple['s'];
      
  $form_state['triples'] = $triples;
  $form_state['values']['title'] = wisski_store_getObj()->wisski_ARCAdapter_addNamespace($mainsubject);
  $form_state['values']['type'] = "individual";
}

function _wisski_pathbuilder_calculate_group_samepart($groupid) {
  if(!$groupid || !isset($groupid)) {
    drupal_set_message("No groupid supplied!", "error");
    return;
  }

  $result = db_query("SELECT * FROM {wisski_template} WHERE id = '$groupid' AND enabled = 1");
  
  $row = db_fetch_array($result);
  
  $samepart = unserialize($row['path_array']);
  
  return $samepart;
    
}

function wisski_pathbuilder_calculate_group_samepart($groupid) {

  if(!$groupid || !isset($groupid)) {
    drupal_set_message("No groupid supplied!", "error");
    return;
  }

  $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = $groupid AND is_group = 0 AND enabled = 1");

  $samepart = array();

  $i = 0;

  while($row = db_fetch_array($result)) {
    if($i == 0)
      $samepart = unserialize($row['path_array']);
    $i++;

    $samepart = array_intersect_assoc($samepart, unserialize($row['path_array']));
  }

  $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = $groupid AND is_group = 1 AND enabled = 1");

  while($row = db_fetch_array($result)) {
    if($i == 0) 
      $samepart = wisski_pathbuilder_calculate_group_samepart($row['id']);
    $i++;

    $subsamepart = wisski_pathbuilder_calculate_group_samepart($row['id']);
    if(count(array_intersect_assoc($samepart, $subsamepart)) == 0) {
      drupal_set_message("Group definition of group '" . $row['name'] . "' is wrong.", 'error');
      return array();
    }
    $samepart = array_intersect_assoc($samepart, $subsamepart);

  }

# from here on bug fix by Mark Fichtner
  $outsamepart = array();
  
  for($i = 0; !empty($samepart['x' . $i]) && !empty($samepart['y' . $i]); $i++) {
    $outsamepart['x' . $i] = $samepart['x' . $i];
    $outsamepart['y' . $i] = $samepart['y' . $i];
  }
  
  if(!empty($samepart['x' . $i]))
    $outsamepart['x' . $i] = $samepart['x' . $i];
  else
    unset($outsamepart['y' . ($i-1)]);
  
  if(count($outsamepart) == 0)
    return array();
  
  if(count($outsamepart) % 2 == 0) {
    unset($outsamepart["y" . ((count($outsamepart) / 2) - 1)]);
  }
  
  return $outsamepart;

}




function wisski_pathbuilder_check_auth_triples($key, $value, $form_state) {
  $authtriples = array();
  $indiv = NULL;
  drupal_load('module', 'wisski_vocab_ctrl');

/* I know where they are... so I don't have to search for them :D  
  # dull search for the auth data field; should be done better
  $agenda = array($form_state);
  $parts = NULL;
  while (!empty($agenda)) {
    $cur = array_shift($agenda);
    if (isset($cur[$key])) {
      $agenda = array($cur[$key]);
      break;
    }
    
    foreach ($cur as $v) {
      if (is_array($v))
        $agenda[] = $v;
    }

  }

  while (!empty($agenda)) {
    $cur = array_shift($agenda);
    if (isset($cur['auth_data_' . $row])) {
      $parts = $cur['auth_data_' . $row];
      break;
    }
    
    foreach ($cur as $v) {
      if (is_array($v))
        $agenda[] = $v;
    }

  }
*/

  $parts = $form_state[$key];

  if(!empty($parts)) {

    $parts = explode(" ", $parts, 4);

    $auth_id = substr($parts[0], 5, -1);
    $indiv = substr($parts[1], 5, -1);
    $x = substr($parts[2], 3, -1);
    
    $input = $parts[3];
    if($input == $value ) {
      $authtriples = wisski_vocab_ctrl_get_triples($auth_id, $indiv);
    } else {
      $indiv = NULL;
    }
  }
  return array($indiv, $authtriples, $x);
}

function _wisski_pathbuilder_gen_triples_of_group($form, &$form_state, $groupid, $subj = "", $samepart = array()) {

  $triples = array();
  $tmptriples = array();

  $paths = array();
  $owlxml;

  $sameMax = 0;

  $i = 0;

  $mysamepart = _wisski_pathbuilder_calculate_group_samepart($groupid);
  
  $disambsubj = FALSE;
  
  $result = db_query("SELECT * FROM {wisski_template} WHERE id = $groupid");
  $row = db_fetch_array($result);
  
  $disambpath = $row['disamb'];
  if($disambpath > 0) {

    $q = wisski_pathbuilder_get_sparql($disambpath, " FILTER ( ?out = '" . $form_state['values'][$row['disamb']] . "' )", FALSE, floor(count($mysamepart)/2));

    $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q, 'rows');

    $disambsubj = $out[0]["x" . floor(count($mysamepart)/2)]; 
  }

  if(count($mysamepart) >= 3) {
    $sameMax = floor(count($mysamepart)/2);
    $oldMax = floor(count($samepart)/2);

    $obj = "";

    for($i=$oldMax; $i<$sameMax; $i++) {
      $trip = array();
      if($subj == "") {

        $subj = wisski_store_getObj()->wisski_ARCAdapter_delNamespace(wisski_store_makePathTitle(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($mysamepart["x$i"])) . wisski_store_getObj()->wisski_Store_getNewUniqueID());

        $trip = array();
        $trip['s'] = $subj;
        $trip['s_type'] = "uri";
        $trip['p'] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type");
        $trip['o'] = $mysamepart["x$i"];
        $trip['o_type'] = "uri";
        $tmptriples[] = $trip;
      }

      if($disambsubj && $i == $sameMax -1)
        $obj = $disambsubj;
      else
        $obj = wisski_store_getObj()->wisski_ARCAdapter_delNamespace(wisski_store_makePathTitle(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($mysamepart[("x". ( $i + 1 ))])) . wisski_store_getObj()->wisski_Store_getNewUniqueID());


      $trip = array();    
      $trip['s'] = $subj;
      $trip['s_type'] = "uri";
      $trip['p'] = $mysamepart["y$i"];
      $trip['o'] = $obj;
      $trip['o_type'] = "uri";
      $tmptriples[] = $trip;

      $trip = array();
      $trip['s'] = $obj;
      $trip['s_type'] = "uri";
      $trip['p'] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type");
      $trip['o'] = $mysamepart[("x" . ( $i + 1 ))];
      $trip['o_type'] = "uri";
      $tmptriples[] = $trip;

      $subj = $obj;  
    }
  } else 
    if($subj == "") {
      $subj = wisski_store_getObj()->wisski_ARCAdapter_delNamespace(wisski_store_makePathTitle(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($mysamepart["x0"])) . wisski_store_getObj()->wisski_Store_getNewUniqueID());
      $trip = array();
      $trip['s'] = $subj;
      $trip['s_type'] = "uri";
      $trip['p'] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace("rdf:type");
      $trip['o'] = $mysamepart["x0"];
      $trip['o_type'] = "uri";
      $tmptriples[] = $trip;
    }

  $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = $groupid AND is_group = 0 AND enabled = 1");
  while($row = db_fetch_array($result)) {
    if(is_array($form_state['values'][$row['id']])) {
      foreach($form_state['values'][$row['id']] as $key => $onechoice) {
        if(!empty($onechoice)) {
          $authtriples = wisski_pathbuilder_check_auth_triples($key, $onechoice, $form_state, $row); 
          if(empty($authtriples)) {
            $triples = array_merge($triples, _wisski_pathbuilder_generate_triples_of_path($form, $onechoice, $row['id'], $subj, $mysamepart));
          } else {
            $path = unserialize($row['path_array']);
            $trip = array();
            $trip['s'] = $subj;
            $trip['s_type'] = "uri";
            $trip['p'] = $path["y" . floor(count($mysamepart)/2)];
            $trip['o'] = $authtriples[0]['s'];
            $trip['o_type'] = "uri";
            $triples[] = $trip;
            $triples = array_merge($triples, $authtriples);
          }
        }
      }
    } else {
      if(!empty($form_state['values'][$row['id']])) {
        $authtriples = wisski_pathbuilder_check_auth_triples($row['id'], $form_state['values'][$row['id']], $form_state, $row);

        if(empty($authtriples)) {
          $triples = array_merge($triples, _wisski_pathbuilder_generate_triples_of_path($form, $form_state['values'][$row['id']], $row['id'], $subj, $mysamepart));
        } else {
          $path = unserialize($row['path_array']);
          $trip = array();
          $trip['s'] = $subj;
          $trip['s_type'] = "uri";
          $trip['p'] = $path["y" . floor(count($mysamepart)/2)];
          $trip['o'] = $authtriples[0]['s'];
          $trip['o_type'] = "uri";
          $triples[] = $trip;
          $triples = array_merge($triples, $authtriples);
        }
      }
    }
  }

  $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = $groupid AND is_group = 1 AND enabled = 1");

  while($row = db_fetch_array($result)) {
    $triples = array_merge($triples, _wisski_pathbuilder_gen_triples_of_group($form, $form_state, $row['id'], $subj, $mysamepart));
  }

  if(count($triples) > 0) 
    $tmptriples = array_merge($tmptriples, $triples);
  else
    return array();

  return $tmptriples;

}

/**
 * The main template-building function. Builds everything that can be seen
 * @param $parentID is the parentID of the field. This renders recursively so this is necessary.
 *        $input can be: 0 for viewing, 1 for adding, 2 for editing
 *        $sparqlcondition is necessary for viewing and editing!
 *				$treetype the type of the tree of the pathbuilder - default: 1 so everything is shown.
 * @author Mark Fichtner
 **/
function _wisski_pathbuilder_recursive_build_group($form_state, $parentID, &$paths, $input = 1, $sparqlcondition = "", $treetype = 1) {  
  
  drupal_add_js(drupal_get_path('module', 'wisski_pathbuilder') . '/wisski_pathbuilder.js');
  $vars = variable_get("wisski_pathbuilder_templateids", array());
  //print_r("build with formstate: " . serialize($form_state));
  global $user;

  //drupal_set_message("parent id is: " . $parentID);

  $resultgroup = db_query("SELECT * FROM {wisski_template} WHERE is_group = 1 AND enabled = 1 AND id = $parentID");
  $resultfields = db_query("SELECT * FROM {wisski_template} WHERE enabled = 1 AND group_id = " . $parentID . " ORDER BY weight ASC");

  $form = array();

  $grouparr = db_fetch_array($resultgroup);

  $samepart = _wisski_pathbuilder_calculate_group_samepart($parentID);

  if(!isset($vars[$user->uid][$samepart["x" . (floor(count($samepart)/2))]]) ||
      ($vars[$user->uid][$samepart["x" . (floor(count($samepart)/2))]] != $parentID && $grouparr['group_id'] == 0)) {
    $vars[$user->uid][$samepart["x" . (floor(count($samepart)/2))]] = $parentID;
    variable_set("wisski_pathbuilder_templateids", $vars);
  }

  if(($grouparr['type'] % $treetype) != 0) {

    if($input == 0)
      return "";
    return array();
  }

  $base = array();
  $basestr = "";
//  if(isset($form_state['values']
//  $my_state = $form_state['
  $elementbase = "";
  $multigroup = 1;
  $is_multi_group = FALSE;

  if(isset($grouparr['id'])) {
  
//    $condition = ("FILTER ( ?x" . (floor(count($samepart)/2)) . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($node->title). "> )");
    
  //$sparql = wisski_pathbuilder_get_sparql($parentID, $sparqlcondition);
    
  //$groups = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql);
  
  //$wrapperarr = array;
  
  
  
  //drupal_set_message(serialize($groups));
  //foreach($groups as $key => $group) {
    $wrapper = "";
    //$wrapper = &$wrapperarr[$key]; 
    //$wrapper .= '<div id="wki-collector">';
    if($input == 0) {
      $pos = floor(count($samepart)/2);
          
      $sparql = wisski_pathbuilder_get_sparql($parentID, $sparqlcondition, FALSE, 0, TRUE, array("?x" . $pos));
      
      $multigroup = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql, 'rows');
    
      drupal_add_js('misc/collapse.js');
      $wrapper .= '<fieldset class="collapsible wki-fieldset-view"><legend>' . $grouparr['name'] . '</legend>'; // <- Schwule schreibweise :P
    } else { //if ($input == 2) { // edit
//      drupal_set_message($grouparr['fieldtype']);
    
      if($grouparr['fieldtype'] != "ontogroup") {
        $form['form'][$grouparr['id']] = array(
          '#collapsed' => FALSE,
          '#type' => 'fieldset',
          '#attributes' => array('class' => 'wki-fieldset-edit wisski_formedit'),
          '#collapsible' => '1',
          '#title' => t($grouparr['name']),
          '#tree' => TRUE,
        );
        $base = &$form['form'][$grouparr['id']];
        $form_state = $form_state[$grouparr['id']];
                
        if($input == 2) {
          $pos = floor(count($samepart)/2);
                    
          $q = wisski_pathbuilder_get_sparql($grouparr['id'], $sparqlcondition, FALSE, 0, TRUE, array("?x" . $pos));
          $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q);
          $results = $out['result']['rows'];

          $grp_val = false;

          foreach($results as $key => $value) {
            for($i = 0; $i<floor(count($samepart)/2); $i++) { 
              $grp_val['x' . $i] = $value['x' . $i];
              $grp_val['y' . $i] = $samepart['y' . $i];
            }
            $grp_val['x' . $i] = $value['x' . $i];
          }
          
//          drupal_set_message(serialize($grp_val)); 	    
          if($grp_val) {
            $paths[$grouparr['id']]['path'] = $grp_val;
            $paths = &$paths[$grouparr['id']]['children'];
          }            
//          drupal_set_message("hier: " . serialize($grp_val));
        }      
//        drupal_set_message("path " . $grouparr['id'] . " is: " . serialize($grp_val));
//        drupal_set_message(serialize($paths));

      } else {

        $is_multi_group = TRUE;
        $grp_val = false;
        $from_db = false;        

  //      drupal_set_message("fs is: " . serialize($form_state));
        
        if(!empty($form_state[$grouparr['id']]))
          $grp_val = $form_state[$grouparr['id']][$grouparr['id']];

        if($input == 2) { // we are in edit mode -> something is there already
          $pos = floor(count($samepart)/2);
          $q = wisski_pathbuilder_get_sparql($grouparr['id'], $sparqlcondition, FALSE, 0, TRUE, array("?x" . $pos));
          $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q);
      
          $results = $out['result']['rows'];

          if(!$grp_val) {
            foreach($results as $key => $value) {
              for($i = 0; $i<floor(count($samepart)/2); $i++) { 
                $grp_val[$key]['x' . $i] = $value['x' . $i];
                $grp_val[$key]['y' . $i] = $samepart['y' . $i];
              }
              $grp_val[$key]['x' . $i] = $value['x' . $i];
              $from_db = true;
            }
          //drupal_set_message(serialize($grp_val)); 	    

          }
        }
        
        //drupal_set_message(htmlentities($q));
        
        $myparents = $parents;
        $myparents[] = $grouparr['id'];
//        drupal_set_message("fs is: " . serialize($form_state[$grouparr['id']]["button"]));

        if(!empty($form_state[$grouparr['id']]["button"])) {
          $keys = array_keys($form_state[$grouparr['id']]["button"]);
          $uuid = str_replace("wisski_pathbuilder_dupl_button_", "", $keys[0]); 
        } else {
          $uuid = wisski_get_uid();
        }
        
        
        
        // generate a duplicate button
        $tmp['wisski_pathbuilder_dupl_button_' . $uuid] = array(
          '#type' => 'image_button',
          '#attributes' => array('class' => 'wki-button-dupl wisski_dupl_multi', 'src' => base_path() . 'sites/all/themes/blendedmalts/pics/ico_add.png',
          'alt' => t('duplicate group with id ' . $grouparr['id'])),
          '#submit' => array('wisski_pathbuilder_dupl_submit'),
          '#ahah' => array(
            'event' => 'click',
            'path' => 'wisski_pathbuilder/duplicate/' . $grouparr['id'], //ahah_helper_path($path_structure),
            'wrapper' => 'wisski_pathbuilder_ontogroup_wrapper_' . $uuid,
            //'wrapper' => 'wisski_pathbuilder_ontogroup_wrapper_' . $grouparr['id'],
          ),
        );
        
        $valArr = array();
        $valArr['button'] = $tmp;
        
        $tmpVal = array();
        $tmpVal['#type']   = 'markup';

        $tmpVal['#prefix'] = '<div id="wisski_pathbuilder_ontogroup_wrapper_' . $uuid . '">'; // $grouparr['id'];
        $tmpVal['#suffix'] = '</div>';
        $tmpVal['#tree'] = TRUE;        

  //      drupal_set_message("grp_val is: " . serialize($grp_val));
  //      drupal_set_message("id: " . $grouparr['id']);
        $countiter = 0;
        
        if(empty($grp_val))
          $grp_val = array($grouparr['id'] . "_0");

        // for every value which is there generate an own group.
        foreach($grp_val as $key => $a_group) {        
//          drupal_set_message("from db is: " . serialize($from_db));
//          drupal_set_message("group is: $key " . serialize($a_group));
          if($from_db) {
            $paths[$grouparr['id'] . "_" . $countiter]['path'] = $a_group;
//            $paths = &$paths[$grouparr['id'] . "_" . $countiter]['children'];
          }
          $tmpVal[$grouparr['id'] . "_" . $countiter] = array(
            '#collapsed' => FALSE,
            '#type' => 'fieldset',
            '#attributes' => array('class' => 'wki-fieldset-edit wisski_formedit'),
            '#collapsible' => '1',
            '#title' => t($grouparr['name']),
            '#tree' => TRUE,
          );
          $countiter++;
        }
        
        $valArr[$grouparr['id']] = $tmpVal;
                
        $form['form'][$grouparr['id']] = $valArr;
         
//        drupal_set_message(serialize($form['form'][$grouparr['id']]));
                                   
        $base = &$form['form'][$grouparr['id']][$grouparr['id']];
        $form_state = $form_state[$grouparr['id']][$grouparr['id']];

        $multigroup = $grp_val;
      }
    }
      /*
    } else {
      $form['form'][$grouparr['id']] = array(
          '#collapsed' => FALSE,
          '#type' => 'fieldset',
          '#attributes' => array('class' => 'wki-fieldset-edit wisski_formedit'),
          '#collapsible' => '1',
          '#title' => t($grouparr['name']),
          '#tree' => TRUE,
          );
      $base = &$form['form'][$grouparr['id']];
      $form_state = $form_state[$grouparr['id']];
    }
    */
  /*
  } else {
    if($input == 0) 
      return "";
    return array();
    $form['form'] = array(
        '#collapsible' => TRUE,
        '#collapsed' => FALSE,
        '#type' => 'fieldset',
        '#tree' => TRUE,
        '#title' => t(''),
        );
    $base = &$form['form'];
  */
  }
  

//  drupal_set_message("input is: " . serialize($paths));

  $iterator = 0;
  $wasHere = array();
  
  $multicount = 1;
  
  if(count($multigroup) > $multicount)
    $multicount = count($multigroup);
  $savepaths = &$paths;    
  for($multiit = 0; $multiit < $multicount; $multiit++) {
  $elementbase = "";  
  
  //drupal_set_message("multigroup is: " . serialize($multigroup));
  
  $pos = floor(count($samepart)/2);

//  drupal_set_message("multigroup is: " . serialize($multigroup[$multiit]));

//  drupal_set_message("pos is: " . $pos);
  
  if($multicount > 1)
    $spqcondition = $sparqlcondition . " . FILTER ( ?x" . $pos . " = <" . $multigroup[$multiit]['x' . $pos] . "> )";
  else
    $spqcondition = $sparqlcondition; 


//  drupal_set_message(htmlentities($spqcondition));
  
  $resultfields = db_query("SELECT * FROM {wisski_template} WHERE enabled = 1 AND group_id = " . $parentID . " ORDER BY weight ASC");
 
//  drupal_set_message("multicount ist: " . serialize($multicount));
  
//  drupal_set_message("base ist: " . serialize($base));
    
  //$tmpbase = &$base;
  
//  drupal_set_message("multicount ist: " . serialize($is_multi_group));

//  drupal_set_message("base before: " . serialize($base));
  
//  $tmpbase = &$base[$grouparr['id']];

//  drupal_set_message("base before: " . serialize($tmpbase));
  
//  drupal_set_message("fs before: " . serialize($form_state));
  
  if(!$is_multi_group) {
    $tmpbase = &$base[$grouparr['id']];
    $fs = $form_state[$grouparr['id']];
  } else if($is_multi_group) {
    $tmpbase = &$base[$grouparr['id'] . '_' . $multiit];
    $fs = $form_state[$grouparr['id'] . '_' . $multiit];
    $paths = &$savepaths[$grouparr['id'] . '_' . $multiit]['children'];  
  }
 // drupal_set_message("after: " . serialize($form_state));
  
//  drupal_set_message("multiit ist: " . $multiit);
  
  while($arr = db_fetch_array($resultfields)) {
    $iterator++;

    if(($arr['type'] % $treetype) > 0)
      continue;

    if($arr['is_group'] == 0) {
      if($input == 1) {  
        //drupal_set_message("Add!!" . serialize($form_state) . "\n has text: " . $text . "\n has id: " . $arr['id']);
        //drupal_set_message(serialize($base[$arr['id']]));
        //drupal_set_message("state: " . serialize($form_state['values'][$arr['id']]));
        $text = $fs[$arr['id']];
        //if(empty($text))
//        $default = $form_state['post'][$arr['id']];

        $tmpbase[$arr['id']] = _wisski_pathbuilder_get_field($arr);
//        $base[$arr['id']]['#default_value'] = $default;
//        drupal_set_message(serialize($base[$arr['id']]));
        $tmpbase['auth_data_' . $arr['id']] = _wisski_pathbuilder_get_autocomplete_data_field($arr);

        if($arr['fieldtype'] == 'checkboxes' || $arr['fieldtype'] == 'select' ) {
        
          $tmpbase[$arr['id']] = _wisski_pathbuilder_get_field($arr);
          $tmpbase['auth_data_' . $arr['id']] = _wisski_pathbuilder_get_autocomplete_data_field($arr);
          $def = array();
                                      
          if(!empty($fs[$arr['id']])) {
            $tmpbase[$arr['id']]['#default_value'] = $fs[$arr['id']];
            $tmpbase['auth_data_' . $arr['id']]['#default_value'] = $fs['auth_data_' . $arr['id']];
          } else {
            $tmpbase[$arr['id']]['#default_value'] = $def;
          }

        } else if( $arr['fieldtype'] == 'ontotext' || $arr['fieldtype'] == 'images' || $arr['fieldtype'] == 'coordinates' ) {

          $tmp = array();
          $tmpVal = array();
          $tmpAuto = array();
          $valArr = array();
          $def = array();


          $tmpVal['#type']   = 'markup';

//          drupal_set_message("here fs is: " . serialize($fs[$arr['id']]['button']));

          if(!empty($fs[$arr['id']]["button"])) {
            $keys = array_keys($fs[$arr['id']]["button"]);
            $uuid = str_replace("wisski_pathbuilder_dupl_button_", "", $keys[0]); 
          } else {
            $uuid = wisski_get_uid();
          }
          
//          drupal_set_message("this is: " . $uuid);

          $tmpVal['#prefix'] = '<div id="wisski_pathbuilder_ontotext_wrapper_' . $uuid . '">'; // $arr['id']
          $tmpVal['#suffix'] = '</div>';
          $tmpVal['#tree'] = TRUE;
          $myparents = $parents;
          $myparents[] = $arr['id'];              

          $tmp['wisski_pathbuilder_dupl_button_' . $uuid] = array(
              '#type' => 'image_button', 
              '#attributes' => array('class' => 'wki-button-dupl wisski_dupl_single', 'src' => base_path() . 'sites/all/themes/blendedmalts/pics/ico_add.png',
              'alt' => t('duplicate path with id ' . $arr['id'])),
        //      '#parents' => $myparents,
              '#submit' => array('wisski_pathbuilder_dupl_submit'),
              '#ahah' => array(
                'event' => 'click',
                'path' => 'wisski_pathbuilder/duplicate/' . $arr['id'], //ahah_helper_path($path_structure),
                'wrapper' => 'wisski_pathbuilder_ontotext_wrapper_' . $uuid, // $arr['id']
                ),
              );


          if(!empty($fs[$arr['id']][$arr['id']])) {
            $cnt = wisski_pathbuilder_filter_auth_data($fs[$arr['id']][$arr['id']]);
            $count = count($cnt);
          } else {
            $count = 1;
          }        
          
          //drupal_set_message("I am here: " . serialize($fs));


          for($tmpi=0; $tmpi<$count; $tmpi++) {
            $tmpVal[$arr['id'] . '_' . $tmpi] = _wisski_pathbuilder_get_field($arr);
            $tmpVal[$arr['id'] . '_auth_data_' . $tmpi] = _wisski_pathbuilder_get_autocomplete_data_field($arr);
            if($tmpi > 0) {
              $tmpVal[$arr['id'] . '_' . $tmpi]['#prefix'] = '<div class="wki-no-label">'; 
              $tmpVal[$arr['id'] . '_' . $tmpi]['#suffix'] = '</div>';
            }
            if(!empty($fs[$arr['id']][$arr['id']][$arr['id'] . '_' . $tmpi])) {
              $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value'] = $fs[$arr['id']][$arr['id']][$arr['id'] . '_' . $tmpi];
//              drupal_set_message("setting authdata!" . serialize($fs));
              $tmpVal[$arr['id'] . '_auth_data_' . $tmpi]['#default_value'] = $fs[$arr['id']][$arr['id']][$arr['id'] . '_auth_data_' . $tmpi];
            } else {
              $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value'] = $out['result']['rows'][$tmpi]['out'];
              $paths[$arr['id'] . '_' . $tmpi]['path'] = $out['result']['rows'][$tmpi];
            }

            $def[$arr['id'] . '_' . $tmpi] = $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value']; 
          }

          $valArr['#default_value'] = $def;
          $valArr['button'] = $tmp;
          $valArr[$arr['id']] = $tmpVal;
//          $valArr[$arr['id'] . '_' . $arr['id']] = $tmpAuto;
          $tmpbase[$arr['id']] = $valArr;
        } else {
/*
          //drupal_set_message("Getting: " .$arr['id'] . "with value: " . $form_state['post'][$arr['id']]);
          if(!isset($form_state['wisskiValues']) || empty($form_state['wisskiValues'][$arr['id']]))
            $tmpbase[$arr['id']]['#default_value'] = $form_state['values'][$arr['id']];
          else if(!isset($form_state['values']) || empty($form_state['values']))
            $tmpbase[$arr['id']]['#default_value'] = $form_state['post'][$arr['id']];
          else
            $tmpbase[$arr['id']]['#default_value'] = $form_state['wisskiValues'][$arr['id']];
          //drupal_set_message("base is: " . serialize($base));
          */
          
          $tmpbase[$arr['id']]['#default_value'] = $fs[$arr['id']];
          $tmpbase['auth_data_' . $arr['id']]['#default_value'] = $fs['auth_data_' . $arr['id']];
        }
      } else if($input == -1) {

        $q = wisski_pathbuilder_get_sparql($arr['id'], $spqcondition);
        $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q);

        $results = $out['result']['rows'];

        $resout = "";
        if($results) {
          $itempath = unserialize($arr['path_array']); 
          foreach($results as $result) {
            if($resout != "")
              $resout .= "<br>";
            for($itera = 0; $itera <= $arr['length']; $itera++) {
              $options = array('attributes' => array('title' => wisski_store_getObj()->wisski_ARCAdapter_addNamespace($itempath["x$itera"]))); 
              if($itera == 0) 
                $resout .= l(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($result["x" . $itera]), $result["x" . $itera], $options);
              else {
                $resout .= " ". l("-->", $itempath["y" . ($itera -1)], array('attributes' => array('title' => wisski_store_getObj()->wisski_ARCAdapter_addNamespace($itempath["y" . ($itera - 1)]))))
                  .   " "   . l(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($result["x" . $itera]), $result["x" . $itera], $options);
              }
            }
        

            if(!isset($arr['datatype_property']) || $arr['datatype_property'] == "") {
              $datatypeprops = wisski_store_getObj()->wisski_ARCAdapter_getDatatypePropertiesForConcept($path[("x" . $arr['length'])]);
              $datatypeprop = $datatypeprops[0];
            } else {
              $datatypeprop = $arr['datatype_property'];
            }
            $resout .= " " . l('-->', $datatypeprop, array('attributes' => array('title' =>wisski_store_getObj()->wisski_ARCAdapter_addNamespace($datatypeprop)))) .
              ' "' . wisski_store_getObj()->wisski_ARCAdapter_addNamespace($result["out"]) . '"';
          }

        }
        $tmpbase[$arr['id']] = array(
            '#type' => 'item',
            '#title' => t($arr['name']),
            '#value' => $resout,
            '#description' => t(''),
            );

        // Edit    
      } else if($input == 2) {

        $q = wisski_pathbuilder_get_sparql($arr['id'], $spqcondition, FALSE, 0, TRUE, array("?out"));

        if(!isset($wasHere[$q]))
          $wasHere[$q] = 0;
        $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q);
        $resultarr = $out['result']['rows'][$wasHere[$q]];
        $result = $out['result']['rows'][$wasHere[$q]]['out'];
        $wasHere[$q] = $wasHere[$q] +1;

        if($arr['fieldtype'] == 'checkboxes' || $arr['fieldtype'] == 'select' ) {

          $tmpbase[$arr['id']] = _wisski_pathbuilder_get_field($arr);
          $tmpbase['auth_data_' . $arr['id']] = _wisski_pathbuilder_get_autocomplete_data_field($arr);
          $def = array();

          
          foreach($out['result']['rows'] as $result1) {
            $def[$result1['out']] = $result1['out'];
          }

/*          $default = false;

          if(!$default && (!isset($form_state['wisskiValues']) || empty($form_state['wisskiValues'][$arr['id']])))
            $default = $form_state['values'][$arr['id']];
          if(!$default && (!isset($form_state['values']) || empty($form_state['values'])))
            $default = $form_state['post'][$arr['id']];
          if(!$default && (isset($form_state['wisskiValues']) && !is_null($form_state['wisskiValues'][$arr['id']])))
            $default = $form_state['wisskiValues'][$arr['id']];
          if(!$default)
            $default = $def;
 */     
          if(!empty($fs[$arr['id']])) {
            $tmpbase[$arr['id']]['#default_value'] = $fs[$arr['id']];
            $tmpbase['auth_data_' . $arr['id']]['#default_value'] = $fs['auth_data_' . $arr['id']];
          } else {
            $tmpbase[$arr['id']]['#default_value'] = $def;
//            drupal_set_message(serialize($out));
            $paths[$arr['id']]['path'] = $out['result']['rows'];
          }
          
 
//          drupal_set_message("setting: " . $base[$arr['id']]['#default_value']);
          
/*          
          if(!empty($form_state['wisskiValues'][$arr['id']]))
            $base[$arr['id']]['#default_value'] = $form_state['wisskiValues'][$arr['id']];
          else
            $base[$arr['id']]['#default_value'] = $def;
*/
        } else if( $arr['fieldtype'] == 'ontotext' || $arr['fieldtype'] == 'images' || $arr['fieldtype'] == 'coordinates'  ) {

          $tmp = array();
          $tmpVal = array();
          $tmpAuto = array();
          $valArr = array();
          $def = array();
//          $uuid = wisski_get_uid();

          if(!empty($fs[$arr['id']]["button"])) {
            $keys = array_keys($fs[$arr['id']]["button"]);
            $uuid = str_replace("wisski_pathbuilder_dupl_button_", "", $keys[0]); 
          } else {
            $uuid = wisski_get_uid();
          }

          $tmpVal['#type']   = 'markup';
          $tmpVal['#prefix'] = '<div id="wisski_pathbuilder_ontotext_wrapper_' . $uuid . '">';
          $tmpVal['#suffix'] = '</div>';
          $tmpVal['#tree'] = TRUE;
          
          $myparents = $parents;
          $myparents[] = $arr['id'];
         
                            

          $tmp['wisski_pathbuilder_dupl_button_' . $uuid] = array(
              '#type' => 'image_button',
              '#attributes' => array('class' => 'wki-button-dupl wisski_dupl_single', 'src' => base_path() . 'sites/all/themes/blendedmalts/pics/ico_add.png',
              'alt' => t('duplicate path with id ' . $arr['id'])),
           //   '#parents' => $myparents,
              '#submit' => array('wisski_pathbuilder_dupl_submit'),
              '#ahah' => array(
                'event' => 'click',
                'path' => 'wisski_pathbuilder/duplicate/' . $arr['id'], //ahah_helper_path($path_structure),
                'wrapper' => 'wisski_pathbuilder_ontotext_wrapper_' . $uuid,
                ),
              );
            
//          drupal_set_message("fs is: " . serialize($fs[$arr['id']]));
          if(!empty($fs[$arr['id']][$arr['id']]) > 0) {
            $cnt = wisski_pathbuilder_filter_auth_data($fs[$arr['id']][$arr['id']]);
            $count = count($cnt);
//            $count = count($fs[$arr['id']][$arr['id']]);
          } else {
            $count = count($out['result']['rows']);
          }
          
          if($count == 0)
            $count = 1;        

          
          //drupal_set_message("I am here: " . serialize($fs));

          for($tmpi=0; $tmpi<$count; $tmpi++) {
            $tmpVal[$arr['id'] . '_' . $tmpi] = _wisski_pathbuilder_get_field($arr);
            $tmpVal[$arr['id'] . '_auth_data_' . $tmpi] = _wisski_pathbuilder_get_autocomplete_data_field($arr);
            if($tmpi > 0) {
              $tmpVal[$arr['id'] . '_' . $tmpi]['#prefix'] = '<div class="wki-no-label">'; 
              $tmpVal[$arr['id'] . '_' . $tmpi]['#suffix'] = '</div>';
            }

            if(!is_null($fs[$arr['id']][$arr['id']][$arr['id'] . '_' . $tmpi])) {
              $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value'] = $fs[$arr['id']][$arr['id']][$arr['id'] . '_' . $tmpi];
              $tmpVal[$arr['id'] . '_auth_data_' . $tmpi]['#default_value'] = $fs[$arr['id']][$arr['id']][$arr['id'] . '_auth_data_' . $tmpi];
            } else {
              $paths[$arr['id'] . '_' . $tmpi]['path'] = $out['result']['rows'][$tmpi];
              $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value'] = $out['result']['rows'][$tmpi]['out'];
            }

            $def[$arr['id'] . '_' . $tmpi] = $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value']; 
          }

          $valArr['#default_value'] = $def;

          $valArr['button'] = $tmp;
          $valArr[$arr['id']] = $tmpVal;
  //        $valArr[$arr['id'] . '_' . $arr['id']] = $tmpAuto;
          $tmpbase[$arr['id']] = $valArr;

        } else {

          $tmpbase[$arr['id']] = _wisski_pathbuilder_get_field($arr);
          $tmpbase['auth_data_' . $arr['id']] = _wisski_pathbuilder_get_autocomplete_data_field($arr);
          
/*          $default = false;

          if(!$default && (!isset($form_state['wisskiValues']) || empty($form_state['wisskiValues'][$arr['id']])))
            $default = $form_state['values'][$arr['id']];
          if(!$default && (!isset($form_state['values']) || empty($form_state['values'])))
            $default = $form_state['post'][$arr['id']];
          if(!$default && (isset($form_state['wisskiValues']) && !is_null($form_state['wisskiValues'][$arr['id']])))
            $default = $form_state['wisskiValues'][$arr['id']];
          if(!$default)
            $default = $result;
 */
 
//           drupal_set_message($fs[$arr['id']] . " : " . $arr['id'] . " : " . serialize($out['result']['rows']));
 
          if(empty($fs[$arr['id']])) {
            $tmpbase[$arr['id']]['#default_value'] = $result;
//            drupal_set_message(serialize($resultarr));
            $paths[$arr['id']]['path'] = $resultarr;
          } else {
            $tmpbase[$arr['id']]['#default_value'] = $fs[$arr['id']];
            $tmpbase['auth_data_' . $arr['id']]['#default_value'] = $fs['auth_data_' . $arr['id']];
          }
        }
        
//        drupal_set_message($arr['id'] . " : " . serialize($out['result']['rows']));

        $tmpbase[$arr['id']]['#disabled'] = $input == 0 ? TRUE : FALSE;

        // view
      } else {
        if($arr['disamb'] > 0)
          $q = wisski_pathbuilder_get_sparql($arr['id'], $spqcondition, FALSE, 0, TRUE, array("?x" . ($arr['disamb']-1) ,"?out"));
        else
          $q = wisski_pathbuilder_get_sparql($arr['id'], $spqcondition, FALSE, 0, TRUE, array("?out"));

//        drupal_set_message("path " . $arr['id'] . " got value: " . htmlentities($q));

        if(!isset($wasHere[$q]))
          $wasHere[$q] = 0;
        $out = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($q);
        
//        drupal_set_message("path " . $arr['id'] . " got value: " . serialize($out));
        
        
        $resultarr = $out['result']['rows'][$wasHere[$q]];
        $result = $out['result']['rows'][$wasHere[$q]]['out'];

        $wasHere[$q] = $wasHere[$q] +1;
        $elementwrap = "";


        if($arr['fieldtype'] == 'checkboxes' || $arr['fieldtype'] == 'select' ) {

          $elementwrap .= '<div class="wki-data-item"><div class="wki-data-label">' . check_plain($arr['name']) . '</div>';
          $elements = "";
          foreach($out['result']['rows'] as $result1) {
            if($arr['disamb'] > 0) { 
              $elements .= '<div class="wki-data-value"><a href="' . $result1[('x' . ($arr['disamb']-1))] . '">' . check_plain($result1['out']) . '</a></div>';
            } else {
              $elements .= '<div class="wki-data-value">' . check_plain($result1['out']) . '</div>';
            }
          }

          if($elements != "")
            $elements = $elementwrap . $elements . '</div>';          

        } else if( $arr['fieldtype'] == 'ontotext' || $arr['fieldtype'] == 'images' || $arr['fieldtype'] == 'coordinates'  ) {
          $elementwrap .= '<div class="wki-data-item"><div class="wki-data-label">' . check_plain($arr['name']) . '</div>';
          $elements = "";

          if(!empty($fs['wisskiValues'][$arr['id']])) {
            $count = count($fs['wisskiValues'][$arr['id']]);
          } else {
            $count = count($out['result']['rows']);
          }        

          for($tmpi=0; $tmpi<$count; $tmpi++) {

            if($arr['disamb'] > 0) {
              if($out['result']['rows'][$tmpi]['out']) {
                $groupids = wisski_pathbuilder_getInternaGroupsForConcept($out['result']['rows'][$tmpi][('x' . ($arr['disamb']-1))]);
                if($groupids && count($groupids) > 0)
                  $elements .= '<div class="wki-data-value"><a href="' . $out['result']['rows'][$tmpi][('x' . ($arr['disamb']-1))] . '">' . check_plain($out['result']['rows'][$tmpi]['out']) . '</a></div>';
                else
                  $elements .= '<div class="wki-data-value">' . check_plain($out['result']['rows'][$tmpi]['out']) . '</div>';
              }
            } else {
              $elements .= '<div class="wki-data-value">' . check_plain($out['result']['rows'][$tmpi]['out']) . '</div>';
            }
            $def[$arr['id'] . '_' . $tmpi] = $tmpVal[$arr['id'] . '_' . $tmpi]['#default_value']; 
          }


          if($elements != "")
            $elements = $elementwrap . $elements . '</div>';

        } else {
          $elements = "";
          $result = check_plain($result);
          if($arr['disamb'] > 0) {
            if($resultarr[('x' . ($arr['disamb']-1))]) {

              $groupids = wisski_pathbuilder_getInternaGroupsForConcept($resultarr[('x' . ($arr['disamb']-1))]);
              if($groupids && count($groupids) > 0)
                $elements .= '<div class="wki-data-item"><div class="wki-data-label">' . check_plain($arr['name']) . '</div>' . '<div class="wki-data-value"><a href="' . $resultarr[('x' . ($arr['disamb']-1))] . '">' . $result . '</a></div></div>';
              else
                $elements .= '<div class="wki-data-item"><div class="wki-data-label">' . check_plain($arr['name']) . '</div>' . '<div class="wki-data-value">' . $result . '</div></div>';
            }
          } else {
            if($result)
              $elements .= '<div class="wki-data-item"><div class="wki-data-label">' . check_plain($arr['name']) . '</div>' . '<div class="wki-data-value">' . $result . '</div></div>';
          }
        }
      }

      $elementbase .= $elements;

    } else {
//      $parents[] = $grouparr['id'];
//      $parents[] = $grouparr['id'];
//      if(!empty($form_state[$grouparr['id']]))
//        $fs = $form_state[$grouparr['id']];
//      if(!empty($fs[$grouparr['id']]))
//        $fs = $fs[$grouparr['id']];
//      if($is_multi_group) {
//        $parents[] = $grouparr['id'] . "_" . $multiit;
//        $fs = $fs[$grouparr['id'] . "_" . $multiit];  
//      }
      $got = _wisski_pathbuilder_recursive_build_group($fs, $arr['id'], $paths, $input, $spqcondition, $treetype);
//      drupal_set_message("getting values from subgroup: " . $arr['id'] . " with fs " . serialize($fs));
      if(count($got) > 0) {

        if($input == 0)
          $elementbase .= $got;
        else
          $tmpbase[$arr['id']] = $got['form'][$arr['id']];

      }
    }
  }

  if($input == 0 && $elementbase)
    $basestr .= $wrapper . $elementbase . '</fieldset>';
  }
  
  if($iterator == 0)
    if($input == 0)
      return "";
    else 	          
      return array();


  if($input == 0) 
    return $basestr; // . '</fieldset>'; // <- Dirty Georg enlarges his string. short!!!!!!!!!!!!

//  drupal_set_message("created form: " . serialize($paths));

  

  return $form;

}

function _wisski_pathbuilder_get_field($arr) {
  $options = array();
  $out = array(  
      '#type' => $arr['fieldtype'], //'textfield',
      '#title' => t($arr['name']),
      '#default_value' => "",
      '#description' => t(''),
      '#size' => $arr['fieldsize'],
      '#attributes' => array('class' => 'wisski_display' . $arr['displaytype'], 'title' => $arr['description']),	
      '#required' => $arr['mandatory'],
      );



  if($arr['fieldtype'] == 'checkboxes') {
    unset($out['#default_value']);
    $out['#options'] = wisski_pathbuilder_getOptionArray($arr['id']);
  } else if($arr['fieldtype'] == 'select') {
    $out['#default_value'] = array();
    $out['#options'] = wisski_pathbuilder_getOptionArray($arr['id']);
    $out['#multiple'] = $arr['repeatable'];
  } else if($arr['fieldtype'] == 'radios') {
    $out['#default_value'] = "";
    $out['#options'] = wisski_pathbuilder_getOptionArray($arr['id']);
  } else if($arr['fieldtype'] == 'ontotext' || $arr['fieldtype'] == 'images' || $arr['fieldtype'] == 'coordinates') {
    $out['#type'] = "textfield";    
    $out['#autocomplete_path'] = ('wisski_pathbuilder/autocomplete/' . $arr['id']);
    $out['#maxlength'] = 2000;
  } else {
    $out['#autocomplete_path'] = ('wisski_pathbuilder/autocomplete/' . $arr['id']);
    $out['#maxlength'] = 2000;
  }


  return $out;
}

function _wisski_pathbuilder_get_autocomplete_data_field() {
  $out = array('#type' => 'hidden', '#default_value' => '');
  return $out;  
}

function _wisski_pathbuilder_add_groups_to_menu() {

  $gotIDs = variable_get('wisski_pathbuilderMenuIDs', array());

  $result = db_query("SELECT * FROM {wisski_template} WHERE is_group = 1 AND enabled = 1 AND group_id = 0");

  $ids = array();
  $data = array();

  while($arr = db_fetch_array($result)) {
    if(($arr['type'] % 2) == 0) {
      $ids[] = $arr['id'];
      $data[] = $arr;
    }
  }  
  
  foreach($gotIDs as $id => $itemid) {
    $oldlinks[$itemid] = menu_link_load($id);
    if(!in_array($itemid, $ids)) 
      menu_link_delete($id);
  }

  $result = db_query("SELECT mlid FROM {menu_links} WHERE link_path = 'node/add' && menu_name = 'navigation'");

  $arr = db_fetch_array($result);

  $mlidIndividual = $arr['mlid'];

  $saveIDs = array();
  foreach($data as $arr) {

    $item = array();
    $item['link_path'] = ("node/add/individual/" . $arr['id']);
    $item['link_title'] = $arr['name'];
    $item['menu_name'] = isset($oldlinks[$arr['id']]['menu_name']) ? $oldlinks[$arr['id']]['menu_name'] : "navigation";
    $item['weight'] = $arr['weight'];
    $item['expanded'] = isset($oldlinks[$arr['id']]['expanded']) ? $oldlinks[$arr['id']]['expanded'] : 0;
    $item['options'] = array(); //isset($oldlinks[$arr['id']]['options']) ? $oldlinks[$arr['id']]['options'] : 0,
    $item['hidden'] = isset($oldlinks[$arr['id']]['hidden']) ? $oldlinks[$arr['id']]['hidden'] : 0;
    $item['mlid'] = isset($oldlinks[$arr['id']]['mlid']) ? $oldlinks[$arr['id']]['mlid'] : NULL;
    $item['plid'] = isset($oldlinks[$arr['id']]['plid']) ? $oldlinks[$arr['id']]['plid'] : $mlidIndividual;

    $id = menu_link_save(&$item);       
    $saveIDs[$id] = $arr['id'];
  }

  variable_set('wisski_pathbuilderMenuIDs', $saveIDs);
  }

  /**
   * Submit handler for the menu overview form.
   *
   * This function takes great care in saving parent items first, then items
   * underneath them. Saving items in the incorrect order can break the menu tree.
   *
   * @see menu_overview_form()
   */
  function wisski_pathbuilder_overview_form_submit($form, &$form_state) {

    $items = &$form_state['values']['wisski_pathbuilder_show'];

    foreach(element_children($form['wisski_pathbuilder_show']) as $mlid) {

      if (isset($form['wisski_pathbuilder_show'][$mlid]['#item'])) {

        $id = $items[$mlid]["mlid"];
        $plid = $items[$mlid]["plid"];

        $data = wisski_pathbuilder_getPathData($id);
        db_query("UPDATE {wisski_template} SET weight = " . $items[$mlid]['weight'] . " , enabled = " . $items[$mlid]['enabled'] . ", group_id = " . $plid . " WHERE id = " . $id);
        if($data['is_group']) {
          $path = wisski_pathbuilder_calculate_group_samepart($id);
          $path_str = serialize($path);
          
          db_query("UPDATE {wisski_template} SET path_array = '%s' WHERE id = '$id'", $path_str);
        
        }
      }
    }
    _wisski_pathbuilder_add_groups_to_menu();

  }


  function wisski_pathbuilder_add_group($form, &$form_state) {
    wisski_pathbuilder_saveToDB(serialize(array()), 0, "", "", $groupid, 1, 1, "textfield", 1, 0, 0, 2);
  }

  function wisski_pathbuilder_calculate_path() {

    $form_state = array('storage' => NULL, 'submitted' => FALSE, 'rebuild' => TRUE);
    $form_build_id = $_POST['form_build_id'];
    $form = form_get_cache($form_build_id, $form_state);

    $args = $form['#parameters'];  
    $form_id = array_shift($args);  
    $form_state['post'] = $form['#post'] = $_POST;  
    $form['#programmed'] = $form['#redirect'] = FALSE;

    $concept = wisski_store_getObj()->wisski_ARCAdapter_delNamespace($form['item']['wisski_pathbuilder_path']['wisski_pathbuilder_add_drop']['#options'][$_POST['item']['wisski_pathbuilder_path']['wisski_pathbuilder_add_drop']]);
    if(count($form_state['storage']['wisski_pathbuilder_path'])&1) {
      $form_state['storage']['wisski_pathbuilder_path'][("y" . (floor(count($form_state['storage']['wisski_pathbuilder_path'])/2)))] = $concept;
    } else {
      $form_state['storage']['wisski_pathbuilder_path'][("x" . (floor(count($form_state['storage']['wisski_pathbuilder_path'])/2)))] = $concept;
    }


    drupal_process_form($form_id, $form, $form_state);
    $form = drupal_rebuild_form($form_id, $form_state, $args, $form_build_id);

    $changed_elements = $form['item']['wisski_pathbuilder_path'];  

    $output = drupal_render($changed_elements);
    drupal_set_header('Content-Type: text/html;');
    echo drupal_to_js(array('status' => true, 'data' => $output));
    exit();

  }

  function wisski_pathbuilder_add_own_path($form, &$form_state) {
    $type = 2;
    if(arg(4))
      $type = arg(4);
    $form_state['redirect'] = 'admin/settings/wisski/pathbuilder/add_own_path/' . $type;
  }

  function _wisski_pathbuilder_add_own_path_submit($form, &$form_state) {

    $type = 2;
    if(arg(5))
      $type = arg(5);
    if(count($form_state['storage']['wisski_pathbuilder_path']) > 0) {
      $max = floor(count($form_state['storage']['wisski_pathbuilder_path'])/2);

      wisski_pathbuilder_saveToDB(serialize($form_state['storage']['wisski_pathbuilder_path']), $max, $form_state['values']['item']['name'], "", 0, -51, 0, "textfield", 0, 0, 1, $type);
    }
    unset($form_state['storage']);
    if($type == 2)
      $form_state['redirect'] = 'admin/settings/wisski/pathbuilder/';
    else
      $form_state['redirect'] = 'admin/settings/wisski/pathbuilder/' . $type . '/';
  }

  function _wisski_pathbuilder_add_own_path(&$form_state, $type = 2) {
    $form['item'] = array(
        '#type' => 'fieldset',
        '#title' => t('Add Item'),
        '#collapsible' => FALSE,
        '#tree' => TRUE,
        '#weight' => -2,
        );

    $form['item']['name'] = array('#type' => 'textfield',
        '#title' => t('Path shortcut name'),
        '#default_value' => isset($form_state['storage']['name']) ? $form_state['storage']['name'] : '',
        '#description' => t(''),
        );

    $form['item']['wisski_pathbuilder_path'] = array(
        '#type' => 'markup',
        '#prefix' => '<div id="wisski_pathbuilder_aownp">',
        '#suffix' => '</div>',
        '#value' => "<div></div>",
        );

    $path = isset($form_state['storage']['wisski_pathbuilder_path']) ? $form_state['storage']['wisski_pathbuilder_path'] : array();

    foreach($path as $key => $element) {
      $form['item']['wisski_pathbuilder_path'][$key]['#value'] = wisski_store_getObj()->wisski_ARCAdapter_addNamespace($element) . '<br>';
    }

    $subclasses = array();

    if(count($path)&1) {
      $subproperties = array(); 

      $classes = wisski_store_getObj()->wisski_ARCAdapter_getUpperClasses(wisski_store_getObj()->wisski_ARCAdapter_delNamespace(
            $form_state['storage']['wisski_pathbuilder_path'][("x" . (floor(count($path)/2)))]));

      foreach($classes as $class) {

        $subp = wisski_reasoner_getPropertiesForConcept(wisski_store_getObj()->wisski_ARCAdapter_delNamespace($class));
        for($iter = 0; $iter < count($subp); $iter++) {
          $subp[$iter] = wisski_store_getObj()->wisski_ARCAdapter_addNamespace($subp[$iter]);
        }

        $subproperties = array_merge($subproperties, $subp);

      }

      $subclasses = $subproperties;
    } else {
      if(count($path) == 0) 
        $subclasses = wisski_store_getObj()->wisski_ARCAdapter_getAllConcepts(); //wisski_ARCAdapter_getSubClasses(wisski_store_getObj()->wisski_ARCAdapter_delNamespace("owl:Thing"));
      else {
        $stringConcepts = wisski_reasoner_getRangeConcepts(wisski_store_getObj()->wisski_ARCAdapter_delNamespace(
              $form_state['storage']['wisski_pathbuilder_path'][("y" . ((floor(count($path)/2)) - 1))]));

        $subclasses = array();

        foreach($stringConcepts as $concept) {
          $subclasses = array_merge($subclasses, wisski_store_getObj()->wisski_ARCAdapter_getSubClasses($concept));
        }

        $subclasses = array_unique($subclasses);
      }
    }
    natcasesort($subclasses);

    $subclasses = array_merge(array("select one"), $subclasses);

    $form['item']['wisski_pathbuilder_path']['wisski_pathbuilder_add_drop'] = array(
        '#default_value' => 0,
        '#key_type' => 'associative',
        '#multiple_toggle' => '1',
        '#type' => 'select',
        '#options' => $subclasses, //($i > 0) ? ( isset($path[("y" . ($i - 1))]) ? $subclasses : array ( 0 => "none")) : $subclasses,
        '#title' => t('Select the next step of the path'),
        '#ahah' => array(
          'event' => 'change',
          'path' => 'admin/settings/wisski/pathbuilder/get_own_path',
          'wrapper' => "wisski_pathbuilder_aownp",
          'method' => "replace",
          'effect' => "fade",
          ),
        );

    $form['submit'] = array('#type' => 'submit', '#value' => t('Save')); //, '#submit' => array('_wisski_pathbuilder_add_own_path_submit'));


    return $form;

  }


/**
 * Theme the menu overview form into a table.
 *
 * @ingroup themeable
 */
function theme_wisski_pathbuilder_overview_form($form) {

  drupal_add_tabledrag('wisski_pathbuilder-overview', 'match', 'parent', 'wisski-plid', 'wisski-plid', 'wisski-mlid', TRUE);
  drupal_add_tabledrag('wisski_pathbuilder-overview', 'order', 'sibling', 'wisski-weight');

  $header = array(
      t('Menu item'),
      t('Path'),
      array('data' => t('Enabled'), 'class' => 'checkbox'),
      t('Weight'),
      array('data' => t('Operations'), 'colspan' => '3'),
      );

  $rows = array();
  foreach (element_children($form['wisski_pathbuilder_show']) as $mlid) {
    if (isset($form['wisski_pathbuilder_show'][$mlid]['enabled'])) {
      $element = &$form['wisski_pathbuilder_show'][$mlid];
      // Build a list of operations.
      $operations = array();
      foreach (element_children($element['operations']) as $op) {
        $operations[] = drupal_render($element['operations'][$op]);
      }
      while (count($operations) < 2) {
        $operations[] = '';
      }

      // Add special classes to be used for tabledrag.js.
      $element['plid']['#attributes']['class'] = 'wisski-plid';
      $element['mlid']['#attributes']['class'] = 'wisski-mlid';
      $element['weight']['#attributes']['class'] = 'wisski-weight';

      // Change the parent field to a hidden. This allows any value but hides the field.
      $element['plid']['#type'] = 'hidden';

      $row = array();
      $row[] = '<div class="wki-col1">' . theme('indentation', $element['#item']['depth'] - 1) . drupal_render($element['title']) . '</div>';
      $row[] = '<span class="wki-col2">' . drupal_render($element['name']) . '</span>';
      $row[] = array('data' => drupal_render($element['enabled']), 'class' => 'checkbox');

      $row[] = drupal_render($element['weight']) . drupal_render($element['plid']) . drupal_render($element['mlid']);
      $row = array_merge($row, $operations);

      $row = array_merge(array('data' => $row), $element['#attributes']);
      $row['class'] = !empty($row['class']) ? $row['class'] .' draggable' : 'draggable';
      $rows[] = $row;
    }
  }

  $output = '';
  if ($rows) {
    $output .= theme('table', $header, $rows, array('id' => 'wisski_pathbuilder-overview'));
  }

  $output .= drupal_render($form);
  return $output;
}


/**
 * Menu callback; Build the menu link editing form.
 */

function wisski_pathbuilder_edit_item(&$form_state, $type, $item, $menu) {

  $result = db_query("SELECT * FROM {wisski_template} WHERE id = " . $item);
  $data = db_fetch_array($result);

  $form['item'] = array(
      '#type' => 'fieldset',
      '#title' => t('Item settings'),
      '#collapsible' => FALSE,
      '#tree' => TRUE,
      '#weight' => -2,
      );

  $form['item']['name'] = array('#type' => 'textfield',
      '#title' => t('Path label'),
      '#default_value' => $data['name'],
      '#description' => t(''),
      );

  if($data['is_group'] == 1) {
    $form['item']['short_name'] = array('#type' => 'textfield',
        '#title' => t('Change group label (syntax !pathnr)'),
        '#default_value' => $data['short_name'],
        '#description' => t(''),
        );
  }

  if($data['is_group'] == 0) {
    $form['item']['mandatory'] = array('#type' => 'checkbox',
        '#title' => t('Mandatory'),
        '#default_value' => $data['mandatory'],
        '#description' => t(''),
        );
  }
  
  $form['item']['description'] = array('#type' => 'textfield',
      '#title' => t('Description of this group or path'),
      '#default_value' => $data['description'],
      '#description' => t(''),
      '#maxlength' => 2000,  
      );


  $form['item']['wisski_pathbuilder_edit_path'] = array(
      '#type' => 'markup',
      '#prefix' => '<div id="wisski_pathbuilder_edit_path">',
      '#suffix' => '</div>',
      '#value' => "<div></div>",
      );

  if($data['is_group'] == 0) 
  $form['item']['fieldtype'] = array(
      '#default_value' => $data['fieldtype'],
      '#key_type' => 'associative',
      '#multiple_toggle' => '1',
      '#type' => 'select',
      '#options' => array("textfield" => "textfield", "textarea" => "textarea", 
        "checkboxes" => "checkboxes", "select" => "select", "radios" => "radios", 
        "ontotext" => "ontotext", "images" => "images", "description" => "description", "coordinates" => "coordinates"),
      '#title' => 'Visualization of the path',
      );

  $types = array();
  if(!($data['type'] % 2))
    $types[] = 2;
  if(!($data['type'] % 3))
    $types[] = 3;
  if(!($data['type'] % 5))
    $types[] = 5;

  $form['item']['type'] = array(
      '#default_value' => $types,

      '#type' => 'checkboxes',
      '#options' => array(2 => "internal display", 3 => "import", 
        5 => "export"),
      '#title' => 'This path or group is used for',
      );


  $path = unserialize($data['path_array']);
  if($data['is_group'] == 1)
    $path = wisski_pathbuilder_calculate_group_samepart($data['id']);
  $max = (round(count($path)/2) -1);

  if($data['is_group'] == 0) {
    $disamboptions = array('no disambiguation', 'user disambiguation');

    for($i = 1; $i < $max+1; $i++) {
      $disamboptions[] = wisski_store_getObj()->wisski_ARCAdapter_addNamespace($path["x" . $i]);	
    }

    $form['item']['disamb'] = array(
      '#default_value' => $data['disamb'],
      '#key_type' => 'associative',
      '#multiple_toggle' => '1',
      '#type' => 'select',
      '#options' => $disamboptions,
      '#title' => 'Select the option for disambiguation',
      );
  } else {
    $disamboptions = array('no disambiguation');
    
    $paths_str = db_query("SELECT * FROM {wisski_template} WHERE group_id = " . $data['id'] ." AND is_group = 0");
    
    while($onepath = db_fetch_array($paths_str)) {
      $disamboptions[$onepath['id']] = "path " . $onepath['id'] . ": " . $onepath['name'];	
    }

    $form['item']['disamb'] = array(
      '#default_value' => $data['disamb'],
      '#key_type' => 'associative',
      '#multiple_toggle' => '1',
      '#type' => 'select',
      '#options' => $disamboptions,
      '#title' => 'Select the option for disambiguation',
      );
      
    $form['item']['fieldtype'] = array(
      '#default_value' => $data['fieldtype'],
      '#key_type' => 'associative',
      '#multiple_toggle' => '1',
      '#type' => 'select',
      '#options' => array("normal group" => "normal group","ontogroup" => "ontogroup"),
      '#title' => 'Select the option for group display',
      );
  }
  
  $form['item']['displaytype'] = array(
      '#default_value' => $data['displaytype'],
      '#key_type' => 'associative',
      '#multiple_toggle' => '1',
      '#type' => 'select',
      '#options' => array("block" => "block", "inline" => "inline"),
      '#title' => 'Select the option for css styling',
      );
  
  $form['item']['fieldsize'] = array(
      '#default_value' => $data['fieldsize'],
      '#type' => 'textfield',
      '#title' => 'Select the size for this field.',
      );

  if(!isset($form_state['storage']['wisski_pathbuilder_edit_path'])) {
    $form['item']['wisski_pathbuilder_edit_path'] = array(
        '#type' => 'fieldset',
        '#title' => t('Path options'),
        '#collapsible' => TRUE,
        '#access' => $data['is_group'] == 1 ? FALSE : TRUE, 
        '#collapsed' => TRUE,
        '#tree' => TRUE,
        '#weight' => 50,
        );

    for($i = 0; $i < $max; $i++) {

      $subclasses = wisski_store_getObj()->wisski_ARCAdapter_getSubClasses($path["x$i"]);

      $form['item']['wisski_pathbuilder_edit_path']["x$i"] = array(
          '#default_value' => 0,
          '#key_type' => 'associative',
          '#multiple_toggle' => '1',
          '#type' => 'select',
          '#options' => $subclasses,
          '#title' => t('Select the ' . ($i+1) . '. concept for the path'),
          );

      $form['item']['wisski_pathbuilder_edit_path']["y$i"] = array(

          '#default_value' => 0,
          '#key_type' => 'associative',
          '#multiple_toggle' => '1',
          '#type' => 'select',
          '#options' => array( wisski_store_getObj()->wisski_ARCAdapter_addNamespace($path["y$i"])),
          '#title' => t('Select the ' . ($i+1) . '. property for the path'),

        );

    }

    if(count($path) > 0) {
      $form['item']['wisski_pathbuilder_edit_path']["x$max"] = array(
          '#default_value' => 0,
          '#key_type' => 'associative',
          '#multiple_toggle' => '1',
          '#type' => 'select',
          '#options' => wisski_store_getObj()->wisski_ARCAdapter_getSubClasses($path["x$max"]),
          '#title' => t('Select the ending concept for the path'),
          );
    }
  }
  if(count($path) > 0 && $data['is_group'] == 0) {
    $dproperties = wisski_store_getObj()->wisski_ARCAdapter_getDatatypePropertiesForConcept($path["x$max"]);

    if(count($dproperties) == 0) {
      drupal_set_message("This path is wrong, " . $path["x$max"] . " may have no datatype property!");
    } else {
      $props = array();
      $nr = 0;
      foreach($dproperties as $dprop) {
        $props[] = wisski_store_getObj()->wisski_ARCAdapter_addNamespace($dprop);
        if($data['datatype_property'] == $dprop) 
          $nr = (count($props) - 1);
      }

      $form['item']['datatype_property'] = array(
          '#default_value' => $nr,
          '#key_type' => 'associative',
          '#multiple_toggle' => '1',
          '#type' => 'select',
          '#options' => $props,
          '#title' => t('Available datatype properties'),
          );
    }
  }

  $form['submit'] = array('#type' => 'submit', '#value' => t('Save'));


  return $form;
}

function _wisski_pathbuilder_generateGroupName($title, $groupid, $data = NULL) {
  $notAnyMatch = TRUE;
  
  if(empty($data)) {
    $result = db_query("SELECT * FROM {wisski_template} WHERE id = " . $groupid);
    $data = db_fetch_array($result);
  }
  
  if($data['short_name'] == "")
    return $title;

  preg_match_all('/(\{.*?){0,1}(![0-9]*)(\}.*?){0,1}/',$data['short_name'], $matches);
//  drupal_set_message(serialize($matches));
  $out = $data['short_name'];

  $samepart = _wisski_pathbuilder_calculate_group_samepart($groupid);

  foreach($matches[2] as $key => $match) {
  
    $sparqlcondition = ("FILTER ( ?x" . (floor(count($samepart)/2)) . " = <" . wisski_store_getObj()->wisski_ARCAdapter_delNamespace($title). "> )");

    $sparql = wisski_pathbuilder_get_sparql(substr($match, 1), $sparqlcondition);

    $sprows = wisski_store_getObj()->wisski_ARCAdapter_getStore()->query($sparql, 'row');
  
    if(!empty($sprows['out'])) {
      if(!empty($matches[1][$key])) {
        $mymatch = str_replace($match, $sprows['out'], $matches[0][$key]); 
        $out = str_replace($matches[0][$key], substr($mymatch, 1, -1), $out);
        $notAnyMatch = FALSE;
      } else {
        $out = str_replace($matches[0][$key], $sprows['out'], $out);
        $notAnyMatch = FALSE;
      }
    } else
      $out = str_replace($matches[0][$key], "", $out);
  }

  if($notAnyMatch)
    return "";

  if(trim($out) == "")
    return $title; 

  return $out;      
}


function wisski_pathbuilder_generateGroupName($title, $groupid, $data = NULL) {
  global $base_url;
  $oldtitle = $title;
  if(!strpos($base_url, $title))
    $title = wisski_store_getObj()->wisski_ARCAdapter_delNamespace($title);

  $result = db_query("SELECT title FROM {wisski_title_cache} WHERE uri = '" . $title . "'");
  
  $title_from_db = db_fetch_array($result);
  
  if(empty($title_from_db) || empty($title_from_db['title'])) {
    $out = _wisski_pathbuilder_generateGroupName(wisski_store_getObj()->wisski_ARCAdapter_addNamespace($title), $groupid, $data);
    
    if(!empty($title_from_db) && empty($title_from_db['title']))
      db_query("UPDATE {wisski_title_cache} SET title = '" . mysql_real_escape_string($out) . "' WHERE uri = '" . mysql_real_escape_string($title) . "'");
    else
      db_query("INSERT INTO {wisski_title_cache} VALUES ('" . mysql_real_escape_string($title) . "', '" . mysql_real_escape_string($out) . "')");
  } else {
    $out = $title_from_db['title'];
  }

  if(trim($out) == "")
    return $oldtitle; 

  return check_plain($out);      
}

/**
 * Process menu and menu item add/edit form submissions.
 */

function wisski_pathbuilder_edit_item_submit($form, &$form_state) {

  $path = array();

  $item = &$form_state['values']['item'];
  
  if(isset($item['wisski_pathbuilder_edit_path']))
    foreach($item['wisski_pathbuilder_edit_path'] as $key => $value) {
      $path[$key] = wisski_store_getObj()->wisski_ARCAdapter_delNamespace($form['item']['wisski_pathbuilder_edit_path'][$key]['#options'][$value]);
    }

  $datatypeproperty = wisski_store_getObj()->wisski_ARCAdapter_delNamespace($form['item']['datatype_property']['#options'][$item['datatype_property']]);

  $id = arg(4);

  $type = 1;

  foreach($item['type'] as $typeitem => $yes)
    if($yes)
      $type = $type * $typeitem;
      
  $result = db_query("SELECT short_name FROM {wisski_template} WHERE id = %d", $id);


  db_query("UPDATE {wisski_template} SET name = '%s', description = '%s', path_array = '%s', " .
      "short_name = '%s', disamb = %d, datatype_property = '%s', mandatory = %d, fieldtype = '%s', type = %d, displaytype = '%s', fieldsize = %d WHERE id = %d", 
      $item['name'], $item['description'], serialize($path), $item['short_name'], $item['disamb'], 
      $datatypeproperty, $item['mandatory'], $item['fieldtype'], $type, $item['displaytype'], $item['fieldsize'], $id);

  $sn = db_fetch_array($result);
  
  if(!empty($item['short_name']) && $sn['short_name'] != $item['short_name'])
    db_query("TRUNCATE TABLE {wisski_title_cache}");

  $form_state['redirect'] = 'admin/settings/wisski/pathbuilder';

}

function wisski_pathbuilder_delete_item($itemnr) {
  return drupal_get_form('wisski_pathbuilder_delete_item_confirm', $itemnr);
}


/**
 * Build a confirm form for deletion of a custom menu.
 */

function wisski_pathbuilder_delete_item_confirm(&$form_state, $itemnr) {
  $form['#items'] = $itemnr;
  $caption .= '<p>'. t('This action cannot be undone.') .'</p>';
  return confirm_form($form, t('Are you sure you want to delete this item (' .$itemnr.')?'), 'admin/settings/wisski/pathbuilder/', $caption, t('Delete'));
}

function _wisski_pathbuilder_delete_item($nr) {
  $result = db_query("SELECT * FROM {wisski_template} WHERE group_id = '$nr'");

  while($row = db_fetch_array($result)) {
    _wisski_pathbuilder_delete_item($row['id']);
  }

  db_query("DELETE FROM {wisski_template} WHERE id = '$nr'");
}

/**
 * Delete a custom menu and all items in it.
 */

function wisski_pathbuilder_delete_item_confirm_submit($form, &$form_state) {
  $itemnr = $form['#items'];
  $form_state['redirect'] = 'admin/settings/wisski/pathbuilder/';

  _wisski_pathbuilder_delete_item($itemnr);
}

function wisski_pathbuilder_get_imagepaths_for_group($id) {

}

/** Returns the components of the sparql query for path identified by $id
 * returns the following hash:
 * 'vars' => array of the variable names. Order is the same as in the path
 * 'triples' => array of sparql triples as strings.
 * 'dtProperty' => a data type property for the last item in the path
 *   *deprecated*
 * 'dtProperties' => an array of possible data type properties for the last item
 */
function wisski_pathbuilder_getSparqlComponents($id) {

  $sparql = wisski_pathbuilder_get_sparql($id, "", TRUE);

  $vars = $sparql['elements'];
  $tmptriples = $sparql['triples'];
  $triples = array();
  unset($tmptriples[count($tmptriples)-1]);
  foreach($tmptriples as $triple) {
    $triples[] = $triple . " . ";
  }
  $dtProperties = array($sparql["primitive_prop"]);

  print_r(array('vars' => $vars, 'triples' => $triples, 'dtProperty' => ($dtProperties ? $dtProperties[0] : NULL) , 'dtProperties' => $dtProperties));  
  return array('vars' => $vars, 'triples' => $triples, 'dtProperty' => ($dtProperties ? $dtProperties[0] : NULL) , 'dtProperties' => $dtProperties);

}

