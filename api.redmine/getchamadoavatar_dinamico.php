<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
require_once 'lib/autoload.php';
error_reporting(E_ERROR);
$vetkanban['tempo_execucao_script']=new DateTime();
// ----------------------------
// Instantiate a redmine client
// --> with ApiKey
$client = new Redmine\Client('http://localhost', '1234567890abcdfgh');

// --> with Username/Password
$client->setCheckSslCertificate(true);
$client->setPort(80);
 

$c=$client->issue->all([       
   'project_id' => 'time-traiket',
'assigned_to_id' => $_GET['user'],  
'limit'=>1
]);

$vetkanban['project_id']='time-traiket';
$vetkanban['qtd_chamados_andamento']=0;

foreach ($c['issues'] as $key => $value) {
    $vetkanban['qtd_chamados_andamento']++;
    $vetkanban['id_chamado_andamento']=$value['id'];            
    $issueId=1; //usar quando não se quer manter os dados por chamados. com isso ira sobrepor os dados no vetor e mantem-se apenas os acumulados

} //fim do foreach dos chamados

    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');
   
echo json_encode($vetkanban);
?>