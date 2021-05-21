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
'query_id' => 8813,  
'limit'=>4000
]);


$vetChamados[]=0;

$vetkanban['project_id']='time-traiket';
$vetkanban['id_filtro_redmine']='8813';
$vetkanban['id_filtro_redmine_tag']='8814';


$qtd_chamados_leadtime=0;
$qtd_chamados_rejeitados=0;
$qtd_chamados_resolvido_fechado=0;
$qtd_chamados_rejeitados_IR=0;

foreach ($c['issues'] as $key => $value) {
    $qtd_chamados_leadtime++;
    $idChamado=$value['id'];            
    $issueId=1; //usar quando não se quer manter os dados por chamados. com isso ira sobrepor os dados no vetor e mantem-se apenas os acumulados

$vetChamados[]=0;
    
    $vetChamado[$issueId]['rejeitado']="";    
    $chamado=$client->issue->show($idChamado , [
    'include' => 'journals',    
    ]);
           

    //Caso precise obter dados da mudança de status
    $tag="";
    foreach ($chamado['issue']['journals'] as $keyJ => $valueJ) {
        foreach ($valueJ['details'] as $keyD => $valueD) {                                         
            if($valueD['property']=='attr' && ($valueD['name']=='status_id' && $valueD['new_value']==41) || ($valueD['name']=='tag_list' && strpos($valueD['new_value'], 'Rejeitado')!==false) )
            { //rejeicao    
                $vetChamado[$issueId]['rejeitado']=$valueJ['created_on'];
            }
        }
    } //fim do fireach dos journals

    //Obter os chamados cancelados indevidamente
    $chamado_rejeitado_indevidamente=0;
    foreach ($chamado['issue']['custom_fields'] as $keyC => $valueC) {       
        if($valueC['name']=='IR' && $valueC['value']=='Sim') {
            $qtd_chamados_rejeitados_IR++;
            $chamado_rejeitado_indevidamente=1;
        }        
    } 

    //Rejeitado
    if(strtotime($vetChamado[$issueId]['rejeitado'])>0 && $chamado_rejeitado_indevidamente==0){        
        $qtd_chamados_rejeitados++;
    }           
} //fim do foreach dos chamados

    
    $vetkanban['qtd_chamados_leadtime']=$qtd_chamados_leadtime;    

    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

    $vetkanban['qtd_chamados_rejeitados']=$qtd_chamados_rejeitados;
    $vetkanban['qtd_chamados_rejeitados_Indevidos']=$qtd_chamados_rejeitados_IR;
    $vetkanban['qtd_chamados_rejeitados_Real']=$qtd_chamados_rejeitados - $qtd_chamados_rejeitados_IR;
    $vetkanban['taxa_rejeicao']=(($qtd_chamados_rejeitados - $qtd_chamados_rejeitados_IR) /$qtd_chamados_leadtime)*100;
    $vetkanban['taxa_rejeicao']=round($vetkanban['taxa_rejeicao'],2); 
    
$c_com_tags=$client->issue->all([       
   'project_id' => 'time-traiket',
'query_id' => 8814,  
'limit'=>2000
]);


$tem_rejeicao=0;
$tem_cancelamento=0;
$tem_impedimento=0;
$tem_Liberado=0;
foreach ($c_com_tags['issues'] as $key => $value) {
    $idChamado=$value['id'];    
    $issueId=1; 
       
    $chamado=$client->issue->show($idChamado , [
    'include' => 'journals',    
    ]);
       
    $tag="";
    foreach ($chamado['issue']['journals'] as $keyJ => $valueJ) {
        foreach ($valueJ['details'] as $keyD => $valueD) {                                                                 

            if($valueD['property']=='attr' && $valueD['name']=='tag_list')
               $tag=$valueD['new_value'];        
        }
    } //fim do fireach dos journals
    if (strpos($tag, 'Rejeitado')!==false) $tem_rejeicao=1;
    if (strpos($tag, 'Cancelamento')!==false) $tem_cancelamento=1;
    if (strpos($tag, 'Impedimento')!==false) $tem_impedimento=1;
    if (strpos($tag, 'Liberado (Em espera)')!==false) $tem_Liberado=1;
    
} //fim do foreach dos chamados
$vetkanban['tem_rejeicao']=$tem_rejeicao;
$vetkanban['tem_cancelamento']=$tem_cancelamento;
$vetkanban['tem_impedimento']=$tem_impedimento;
$vetkanban['tem_Liberado']=$tem_Liberado;


echo json_encode($vetkanban);
?>