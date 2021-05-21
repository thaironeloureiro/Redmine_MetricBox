<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
require_once 'lib/autoload.php';
error_reporting(E_ERROR);

$vetkanban['tempo_execucao_script']=new DateTime();

$client = new Redmine\Client('http://localhost', '1234567890abcdfgh');

$client->setCheckSslCertificate(true);
$client->setPort(80);


$subproject_id=$_GET['subproject_id']?$_GET['subproject_id']:'';

$p=$client->project->all([       
   'project_id' => 'time-traiket',
   'status_id'=>'open',
   'limit'=>4000
]);


$p=$client->project->listing();

echo "Projetos: <pre>";
print_r($p);
echo "</pre>";
?>