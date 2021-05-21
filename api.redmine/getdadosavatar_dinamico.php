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
'from'=>'2019-10-20',
'to'=>'2019-11-21',   
'user_id'=>$_GET['user'],
'limit'=>4000
]);

$qtd_chamados_leadtime=0;
$vetkanban['project_id']='time-traiket';
$vetkanban['DataIni']=$data_ini;
$vetkanban['DataFim']=$data_fim;
$Dtx='01';


$data_aux=$data_ini;
while($data_aux<=$data_fim){
    $vetkanban2['ByDev']['datas'][$data_aux]=0;
    $data_aux = new DateTime($data_aux);
    $data_aux->add(new DateInterval('P1D'));
    $data_aux= $data_aux->format('Y-m-d');
}

foreach ($c['time_entries'] as $key => $value) {
    $data_ref = $value['spent_on'];
    $user = $value['user']['name'];
    $vetkanban['ByDev']['username']=$user;
    $vetkanban2['ByDev']['datas'][$data_ref]=$value['hours'];
    $vetkanban['ByDev']['tothoras']+=$value['hours'];
} 

$ind=0;
foreach ($vetkanban2['ByDev']['datas'] as $keyDt => $value) { 
    $vetkanban['ByDev']['QtdDias']=count($vetkanban2['ByDev']['datas']);        
    
    if($value==0){
        $d=date('w', strtotime($keyDt)); 
        if($d!=0 && $d!=6){
            $vetkanban['ByDev']['DatasNaoAlocadas'][$ind]=$keyDt;
            $vetkanban['ByDev']['QtdDiasNaoAlocados']+=1;
            $ind++;
        }
    }
    else
        $vetkanban['ByDev']['QtdDiasAlocados']+=1;
    $vetkanban['ByDev']['QtdDias']+=1;    
} 


    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);    
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');
echo json_encode($vetkanban);
?>