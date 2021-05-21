<?php header ('Content-type: text/html; charset=UTF-8'); ?>
<?php
error_reporting(E_ERROR);
require_once 'lib/autoload.php';

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
$vetkanban['leadtime_dias']='';
$vetkanban['leadtime']=0;
$vetkanban['cycletime_desenvolvimento']=0;
$vetkanban['cycletime_novo_aberto']=0;
$vetkanban['cycletime_homologacao']=0;
$vetkanban['cycletime_resolvido_fechado']=0;

$qtd_chamados_leadtime=0;
$qtd_chamados_novo_resolvido=0;
$qtd_chamados_novo_aberto=0;
$qtd_chamados_homologa_validado=0;
$qtd_chamados_rejeitados=0;
$qtd_chamados_resolvido_fechado=0;
$qtd_chamados_rejeitados_IR=0;

foreach ($c['issues'] as $key => $value) {
    $qtd_chamados_leadtime++;
    $idChamado=$value['id'];            
    $issueId=1; //usar quando não se quer manter os dados por chamados. com isso ira sobrepor os dados no vetor e mantem-se apenas os acumulados

$vetChamados[]=0;

    $vetChamado[$issueId]['fechado']=""; 
    $vetChamado[$issueId]['resolvido']=""; 
    $vetChamado[$issueId]['aberto']=""; 
    $vetChamado[$issueId]['validado']="";
    $vetChamado[$issueId]['rejeitado']="";
    $vetChamado[$issueId]['created_on']=$value['created_on']; 
    $vetChamado[$issueId]['updated_on']=$value['updated_on']; 
    $vetChamado[$issueId]['closed_on']=$value['closed_on']; 
    
    //leadtime em dias
    $date1= new DateTime(substr($vetChamado[$issueId]['created_on'],0,10));
    $date2= new DateTime(substr($vetChamado[$issueId]['closed_on'],0,10));
    $dif_date = date_diff($date1, $date2,true);    
    $vetkanban['leadtime_dias']+=$dif_date->format('%a');
    $vetkanban['leadtime']+=strtotime($value['closed_on']) - strtotime($value['created_on']);
    
    
    $chamado=$client->issue->show($idChamado , [
    'include' => 'journals',    
    ]);
    

    //Caso precise obter dados da mudança de status
    $tag="";
    foreach ($chamado['issue']['journals'] as $keyJ => $valueJ) {
        foreach ($valueJ['details'] as $keyD => $valueD) {                             
            //status resolvido
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==3 || $valueD['new_value']==115) )
            { 
                $vetChamado[$issueId]['resolvido']=$valueJ['created_on'];
            }

            //status aberto
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==2 || $valueD['new_value']==119) )
            { 
                $vetChamado[$issueId]['aberto']=$valueJ['created_on'];
            }

            //Homologação
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==28 || $valueD['new_value']==116) ) 
            { 
                $vetChamado[$issueId]['homologacao']=$valueJ['created_on'];
            }

            //Validado
            if($valueD['property']=='attr' && $valueD['name']=='status_id' && ($valueD['new_value']==20 || $valueD['new_value']==117) )
            { 
                $vetChamado[$issueId]['validado']=$valueJ['created_on'];
            }

            if($valueD['property']=='attr' && $valueD['name']=='status_id' && $valueD['new_value']==5)
            { 
                $vetChamado[$issueId]['fechado']=$valueJ['created_on'];
            }

            if($valueD['property']=='attr' && ($valueD['name']=='status_id' && $valueD['new_value']==41) || ($valueD['name']=='tag_list' && strpos($valueD['new_value'], 'Rejeitado')!==false) )
            { //rejeicao    
                //$qtd_chamados_rejeitados++;
                $vetChamado[$issueId]['rejeitado']=$valueJ['created_on'];
            }
        }
    } //fim do fireach dos journals
    //if($tag=='Rejeitado') $tem_rejeicao=1;

     

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

    //cycletime desenvolvimento [novo - resolvido] em dias, horas, min e ss
    if(strtotime($vetChamado[$issueId]['resolvido'])>0){
        $vetkanban['cycletime_desenvolvimento']+=(strtotime($vetChamado[$issueId]['resolvido']) - strtotime($value['created_on']));
        $qtd_chamados_novo_resolvido++;                
    }

    if(strtotime($vetChamado[$issueId]['aberto'])>0){
        $vetkanban['cycletime_novo_aberto']+=(strtotime($vetChamado[$issueId]['aberto']) - strtotime($value['created_on']));
        $qtd_chamados_novo_aberto++;                
    }

    //cycletime homologação [homologação - validado] em dias, horas, min e ss
    if(strtotime($vetChamado[$issueId]['validado'])>0 && strtotime($vetChamado[$issueId]['homologacao'])>0){
        $vetkanban['cycletime_homologacao']+=(strtotime($vetChamado[$issueId]['validado']) - strtotime($vetChamado[$issueId]['homologacao']));
        $qtd_chamados_homologa_validado++;        
    }

    //cycletime homologação [resolvido - fechado] em dias, horas, min e ss
    if(strtotime($vetChamado[$issueId]['resolvido'])>0 ){
        $vetkanban['cycletime_resolvido_fechado']+=(strtotime($value['closed_on']) - strtotime($vetChamado[$issueId]['resolvido']));
        $qtd_chamados_resolvido_fechado++;        
    }   
} //fim do foreach dos chamados

    $vetkanban['leadtime']=abs($vetkanban['leadtime']/$qtd_chamados_leadtime);
    $datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['leadtime']));
    $dif_datetime = date_diff($datetime1, $datetime2,true);    
    $vetkanban['leadtime']=$dif_datetime->format("%a dias %hh %i' ");    
    $vetkanban['leadtime_dias']=round(abs($vetkanban['leadtime_dias']/$qtd_chamados_leadtime),2);



    $vetkanban['cycletime_desenvolvimento']=abs($vetkanban['cycletime_desenvolvimento']/$qtd_chamados_novo_resolvido);
    $datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['cycletime_desenvolvimento']));
    $dif_datetime = date_diff($datetime1, $datetime2,true);    
    $vetkanban['cycletime_desenvolvimento']=$dif_datetime->format("%a dias %hh %i' ");

    $vetkanban['cycletime_novo_aberto']=abs($vetkanban['cycletime_novo_aberto']/$qtd_chamados_novo_aberto);
    $datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['cycletime_novo_aberto']));
    $dif_datetime = date_diff($datetime1, $datetime2,true);    
    $vetkanban['cycletime_novo_aberto']=$dif_datetime->format("%a dias %hh %i' ");
    $vetkanban['cycletime_homologacao']=abs($vetkanban['cycletime_homologacao']/$qtd_chamados_homologa_validado);
    $datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['cycletime_homologacao']));
    $dif_datetime = date_diff($datetime1, $datetime2,true);    
    $vetkanban['cycletime_homologacao']=$dif_datetime->format("%a dias %hh %i' ");

    $vetkanban['cycletime_resolvido_fechado']=abs($vetkanban['cycletime_resolvido_fechado']/$qtd_chamados_resolvido_fechado);
    $datetime1=new DateTime(gmdate('Y-m-d h:i:sa',strtotime('1970-01-01T00:00:00Z')));
    $datetime2=new DateTime(gmdate('Y-m-d h:i:sa',$vetkanban['cycletime_resolvido_fechado']));
    $dif_datetime = date_diff($datetime1, $datetime2,true);    
    $vetkanban['cycletime_resolvido_fechado']=$dif_datetime->format("%a dias %hh %i' ");

    $vetkanban['qtd_chamados_leadtime']=$qtd_chamados_leadtime;
    $vetkanban['qtd_chamados_novo_resolvido']=$qtd_chamados_novo_resolvido;
    $vetkanban['qtd_chamados_homologa_validado']=$qtd_chamados_homologa_validado;
    $vetkanban['qtd_chamados_resolvido_fechado']=$qtd_chamados_resolvido_fechado;
    

    $dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

    $vetkanban['qtd_chamados_rejeitados']=$qtd_chamados_rejeitados;
    $vetkanban['qtd_chamados_rejeitados_Indevidos']=$qtd_chamados_rejeitados_IR;
    $vetkanban['qtd_chamados_rejeitados_Real']=$qtd_chamados_rejeitados - $qtd_chamados_rejeitados_IR;
    $vetkanban['taxa_rejeicao']=(($qtd_chamados_rejeitados - $qtd_chamados_rejeitados_IR) /$qtd_chamados_leadtime)*100;
    $vetkanban['taxa_rejeicao']=round($vetkanban['taxa_rejeicao'],2)."%";
    
    
    
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