<?php
include dirname(__FILE__) . '/_init.php';

$params = array(
    'gamma' => 0.7
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-gamma-1.png');

$params = array(
    'gamma' => 1.4
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-gamma-2.png');
