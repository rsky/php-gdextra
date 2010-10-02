<?php
if (!extension_loaded('gdextra')) {
    if (!dl('gdextra.' . PHP_SHLIB_SUFFIX)) {
        exit(1);
    }
}

chdir(dirname(__FILE__));

include_once './_functions.php';
