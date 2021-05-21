<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
require_once 'lib/autoload.php';
error_reporting(E_ERROR);
$vetkanban['tempo_execucao_script']=new DateTime();
// ----------------------------
// Instantiate a redmine client
// --> with ApiKey
$client = new Redmine\Client('http://localhost', '1234567890abcdfgh');

// ----------------------------
// [OPTIONAL] if you want to check
// the servers' SSL certificate on Curl call
$client->setCheckSslCertificate(true);

// ----------------------------
// [OPTIONAL] set the port
// (it will try to guess it from the url)
$client->setPort(80);
 


$c=$client->issue->all([       
   'project_id' => 'time-traiket',
'query_id' => 8813,  
'limit'=>4000
]);


$qtd_chamados_leadtime=0;

$data_fim = date('W', strtotime("now"));
$data_fim=$data_fim-1; if($data_fim<1) $data_fim=1;
$data_ini = $data_fim-10;
if($data_ini<1)  $data_ini=1;

$vetkanban['project_id']='time-traiket';
$vetkanban['id_filtro_redmine']='8813';
$vetkanban['data_ini']=$data_ini;


foreach ($c['issues'] as $key => $value) {
    $qtd_chamados_leadtime++;
    //$idChamado=$value['id'];        
        
    //leadtime em dias
    $date1= new DateTime(substr($value['created_on'],0,10));
    $date2= new DateTime(substr($value['closed_on'],0,10));
    $dif_date = date_diff($date1, $date2,true);        

    $data_ref = date('W', strtotime($value['closed_on']));

    $vetkanban['leadtime'][$data_ref]['qtd_chamados']+=1;
    $vetkanban['leadtime'][$data_ref]['leadtime_dias']+=$dif_date->format('%a');
} //fim do foreach dos chamados

 
foreach ($vetkanban['leadtime'] as $keyDt => $valueL) { 
   if($keyDt>0)    
        $vetkanban['leadtime'][$keyDt]['leadtime_dias']=round(abs($vetkanban['leadtime'][$keyDt]['leadtime_dias']/$vetkanban['leadtime'][$keyDt]['qtd_chamados']),2);    
} 
    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);    
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');
echo json_encode($vetkanban);
?>