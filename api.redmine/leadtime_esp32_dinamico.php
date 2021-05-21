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
'query_id' => 9406,  
'limit'=>4000
]);

$vetChamados[]=0;
$vetkanban['leadtime_medio']=0;

$qtd_chamados_leadtime=0;

foreach ($c['issues'] as $key => $value) {
    $qtd_chamados_leadtime++;
    $idChamado=$value['id'];        
       
    //leadtime em dias
    $date1= new DateTime(substr($value['created_on'],0,10));
    $date2= new DateTime(substr($value['closed_on'],0,10));
    $dif_date = date_diff($date1, $date2,true);        

    $vetkanban['qtd_chamados']=$qtd_chamados_leadtime;
    $vetkanban['leadtime_medio']+=$dif_date->format('%a');
      
            
} //fim do foreach dos chamados
    
    $vetkanban['leadtime_medio']=round(abs($vetkanban['leadtime_medio']/$qtd_chamados_leadtime),2);
    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%a dias %hhs %imin %sseg');
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

   
echo json_encode($vetkanban);
?>