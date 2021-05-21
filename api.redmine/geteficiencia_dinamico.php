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
 


$c=$client->issue->all([       
   'project_id' => 'time-traiket',
'query_id' => 9406,  
'limit'=>4000
]);

$vetkanban['project_id']='time-traiket';
$vetkanban['id_filtro_redmine']='9406';
$vetkanban['leadtime']='';
$vetkanban['waitingtime']='';
$vetkanban['touchtime']='';
$vetkanban['flow_efficiency']='';
$vetkanban['flow_efficiency_numeric']='';

$qtd_chamados_leadtime=0;
$qtd_chamados_novo_aberto=0;
$qtd_chamados_homologa_validado=0;
$qtd_chamados_rejeitados=0;
$qtd_chamados_aguardando_informacao=0;

$atribuido_para_ini='';
$atribuido_para_fim='';

foreach ($c['issues'] as $key => $value) {
    $qtd_chamados_leadtime++;
    $idChamado=$value['id'];            
    $issueId=1; //usar quando não se quer manter os dados por chamados. com isso ira sobrepor os dados no vetor e mantem-se apenas os acumulados

    $vetkanban['leadtime']+=strtotime($value['closed_on']) - strtotime($value['created_on']);
    
    
    $chamado=$client->issue->show($idChamado , [
    'include' => 'journals',    
    ]);
    
       
    foreach ($chamado['issue']['journals'] as $keyJ => $valueJ) {
        foreach ($valueJ['details'] as $keyD => $valueD) {                             
            //chamado sem atribuição e que foi atribuido a algum DEV
            if($valueD['property']=='attr' && $valueD['name']=='assigned_to_id' && $valueD['new_value']>0 && !$valueD['old_value']) 
            {                                               
                $atribuido_para_ini=$valueJ['created_on'];
                    
            }
                    
            //chamado atribuido e que foi teve sua atribuição retirada
            if($valueD['property']=='attr' && $valueD['name']=='assigned_to_id' && !$valueD['new_value'] && $valueD['old_value']>0 && $atribuido_para_ini)  
            { 
                
                $atribuido_para_fim=$valueJ['created_on'];   
                    
                $vetkanban['touchtime']+=(strtotime($atribuido_para_fim) - strtotime($atribuido_para_ini));
                                        
                $atribuido_para_ini='';
                $atribuido_para_fim='';
            }            
        }
    } //fim do fireach dos journals    
} //fim do foreach dos chamados
    $vetkanban['qtd_chamados_leadtime']=$qtd_chamados_leadtime;
    
    $vetkanban['waitingtime']=abs($vetkanban['leadtime']-$vetkanban['touchtime']);

    $vetkanban['flow_efficiency_numeric']= round(($vetkanban['touchtime'] / $vetkanban['leadtime'])*100,2);  
    $vetkanban['flow_efficiency']= $vetkanban['flow_efficiency_numeric']."%";  
    $vetkanban['leadtime']=abs($vetkanban['leadtime']/$qtd_chamados_leadtime);
    $vetkanban['touchtime']=abs($vetkanban['touchtime']/$qtd_chamados_leadtime);
    $vetkanban['waitingtime']=abs($vetkanban['waitingtime']/$qtd_chamados_leadtime);

    $datetimeREF=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
    
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['leadtime']));
    $dif_datetime = date_diff($datetimeREF, $datetime2,true);    
    $vetkanban['leadtime']=$dif_datetime->format('%a dias %hhs %imin %sseg');
    
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['touchtime']));
    $dif_datetime = date_diff($datetimeREF, $datetime2,true);    
    $vetkanban['touchtime']=$dif_datetime->format('%a dias %hhs %imin %sseg');
    
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['waitingtime']));
    $dif_datetime = date_diff($datetimeREF, $datetime2,true);    
    $vetkanban['waitingtime']=$dif_datetime->format('%a dias %hhs %imin %sseg');

    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

echo json_encode($vetkanban);
?>