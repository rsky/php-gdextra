<?php
include dirname(__FILE__) . '/_init.php';

$params = array(
    'levels' => array(51, 204)
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-levels-1.png');

$params = array(
    'levels' => array(0, 255, 51, 204)
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-levels-2.png');
