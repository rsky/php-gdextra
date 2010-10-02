<?php
include dirname(__FILE__) . '/_init.php';

$params = array(
    'tonecurve' => array(
        array(0.5, 0.75)
    )
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-tonecurve-1.png');

$params = array(
    'tonecurve2' => array(
        array(0.3, 0.2), array(0.5, 0.4), array(0.7, 0.8)
    )
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-tonecurve-2.png');

$params = array(
    'tonecurve' => array(
        array(0.2, 0.5), array(0.5, 0.5), array(0.8, 0.5)
    )
);

$graph = generate_graph($params);
imagepng($graph, 'output/graph-tonecurve-3.png');
