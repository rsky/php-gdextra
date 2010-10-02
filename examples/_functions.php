<?php

function generate_grayscale()
{
    $im = imagecreatetruecolor(256, 32);
    for ($i = 0; $i < 256; $i++) {
        $c = ($i << 16) | ($i << 8) | $i;
        imagefilledrectangle($im, $i, 0, $i + 1, 32, $c);
    }
    return $im;
}

function generate_colorbar()
{
    $im = imagecreatetruecolor(256, 32);
    for ($i = 0; $i < 256; $i++) {
        $r = $i << 16;
        $g = $i << 8;
        $b = $i;
        $c = $r | $g | $b;
        imagefilledrectangle($im, $i,  0, $i + 1,  8, $c);
        imagefilledrectangle($im, $i,  8, $i + 1, 16, $r);
        imagefilledrectangle($im, $i, 16, $i + 1, 24, $g);
        imagefilledrectangle($im, $i, 24, $i + 1, 32, $b);
    }
    return $im;
}

function generate_graph($params)
{
    $in = generate_grayscale();
    $out = imageclone($in);

    imagecolorcorrect($out, $params);

    $graph = imagecreate(256, 256);
    imagecolorallocate($graph, 255, 255, 255);
    $z = imagecolorallocate($graph, 0, 0, 0);

    for ($i = 0; $i < 256; $i++) {
        $x = 255 & imagecolorat($in, $i, 0);
        $y = 255 & imagecolorat($out, $i, 0);
        imagesetpixel($graph, $x, 255- $y, $z);
    }

    return $graph;
}
