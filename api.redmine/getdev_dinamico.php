<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
require_once 'lib/autoload.php';
error_reporting(E_ERROR);
$vetkanban['tempo_execucao_script']=new DateTime();
// ----------------------------
// Instantiate a redmine client
// --> with ApiKey
$client = new Redmine\Client('http://localhost', '1234567890abcdfgh');

$client->setCheckSslCertificate(true);
$client->setPort(80);
 



$today = new DateTime();
$AnoIni=$today;
$MesAtual=$today;

$AnoIni=$AnoIni->format('Y');
$AnoFim=$AnoIni;
$MesAtual=$MesAtual->format('m');
$Dia=$today->format('d');

$MesAnt=$MesAtual;

if($Dia>20) {
    if($MesAtual+0 > 1)
    {
        $MesAnt--;
        $MesAnt=str_pad($MesAnt, 2, "0", STR_PAD_LEFT);            
    }else{
        $MesAnt='12';
        $AnoIni--;        
    }

    $data_ini=$AnoIni.'-'.$MesAnt.'-20';
    $data_fim=$AnoFim.'-'.$MesAtual.'-21';    
}
else{
    if($MesAtual+0 > 1) {$MesAtual--; $MesAnt--;}
    else { $MesAtual='12';
           $AnoFim--;
           $AnoIni--;
           $MesAnt=$MesAtual;
    }

    if($MesAtual+0 > 1)
    {
        $MesAnt--;
        $MesAnt=str_pad($MesAnt, 2, "0", STR_PAD_LEFT);            
    }else{
        $MesAnt='12';
        $AnoIni--;        
    }

    $data_ini=$AnoIni.'-'.$MesAnt.'-20';
    $data_fim=$AnoFim.'-'.$MesAtual.'-21';    
}


$c=$client->time_entry->all([       
   'project_id' => 'time-traiket',
'from'=>'$data_ini',
'to'=>'$data_fim',   
'user_ids'=>[1105,631,1537,1063,939,334,328,570,709],             
'limit'=>4000
]);
$qtd_chamados_leadtime=0;


$vetkanban['project_id']='time-traiket';
$vetkanban['id_filtro_redmine']='8813';
$vetkanban['DataIni']=$data_ini;
$vetkanban['DataFim']=$data_fim;
$Dtx='01';
$vetkanban['DiaDev']=0;
$vetkanban['TotalHoras']=0;


$data_aux=$data_ini;
while($data_aux<=$data_fim){
    $vetkanban['ByDate'][$data_aux]['tothoras']=0;
    $vetkanban['ByDate'][$data_aux]['QtdDevs']=0;
    $vetkanban['ByDate'][$data_aux]['Devs']=array();

    $data_aux = new DateTime($data_aux);
    $data_aux->add(new DateInterval('P1D'));
    $data_aux= $data_aux->format('Y-m-d');
}

foreach ($c['time_entries'] as $key => $value) {
    $data_ref = $value['spent_on'];
    $user = $value['user']['name'];
    $vetkanban['ByDev'][$user]['datas'][$data_ref]=$value['hours'];
    $vetkanban['ByDev'][$user]['tothoras']+=$value['hours'];
    $vetkanban['TotalHoras']+=$value['hours'];
    $vetkanban['ByDate'][$data_ref]['tothoras']+=$value['hours'];
    $vetkanban['ByDate'][$data_ref]['Devs'][$user]=$value['hours'];
} //fim do foreach dos chamados

 
foreach ($vetkanban['ByDev'] as $keyUser => $value) { 
        $vetkanban['ByDev'][$keyUser]['QtdDias']=count($vetkanban['ByDev'][$keyUser]['datas']);        
} 

foreach ($vetkanban['ByDate'] as $keyDt => $value) { 
        $vetkanban['ByDate'][$keyDt]['QtdDevs']=count($vetkanban['ByDate'][$keyDt]['Devs']);
        $vetkanban['DiaDev']+=count($vetkanban['ByDate'][$keyDt]['Devs']);
}


    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);    
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

$vetkanban2['tempo_execucao_script']=$vetkanban['tempo_execucao_script'];
$vetkanban2['DiaDev']=$vetkanban['DiaDev'];
$vetkanban2['TotalHoras']=$vetkanban['TotalHoras'];
$vetkanban2['DataIni']=$vetkanban['DataIni'];
$vetkanban2['DataFim']=$vetkanban['DataFim'];


echo json_encode($vetkanban2);
?>