<?php
header("Content-Type: text/html; charset=UTF-8");
ob_start();
phpinfo();
$html = ob_get_clean();
echo $html;
?>
