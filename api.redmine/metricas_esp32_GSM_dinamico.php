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
 

//Listar Status
/*
$client->issue_status->all();
$st=$client->issue_status->listing();
echo "<pre>";
print_r($st);
echo "</pre>";
*/
/*listas de status:
    [Novo] => 1
    [Aberto] => 2
    [Andamento] => 4
    [Pendente] => 10
    [Em desenvolvimento] => 12
    [Resolvido] => 3
    [Homologacao] => 28
    [Validado] => 20
    [Avaliar Producao] => 56
    [Rejeitado] => 41
    [Aguardando Informacao] => 22
    [Cancelado] => 6
    
    [Andamento - VAL] => 76
    [Rejeitado - VAL] => 77
    [Aberto - HML] => 78
    [Desenvolvimento: A Fazer] => 88
    [Desenvolvimento: Fazendo] => 89
    [Desenvolvimento: Feito] => 90
    
    [Homologação: Fazendo] => 91
    [Homologação: Feito] => 92
    
    [SD:Prospect:Novo] => 99
    [SD:Prospect:Analisando] => 100
    [SD:Prospect:Pronto] => 111
    [SD:Dev:A fazer] => 102
    [SD:Dev:Fazendo] => 103
    [SD:Dev:Testando] => 105
    [SD:Dev:Pronto] => 112
    [SD:Dev:Homologando] => 106
    [SD:Dev:Disponível] => 113
    [SD:Deploy:Testando Integrado] => 107
    [SD:Deploy:Publicado] => 108
    [SD:OA:Acompanhando] => 109
    [SD:OA:Feito] => 104
    [SD:Cancelado] => 110
    [Backlog: Novo] => 120
    [Backlog: Aberto] => 119
    
    [Desenvolvimento: Em Andamento] => 114
    [Desenvolvimento: Resolvido] => 115
    [Homologação: Em Andamento] => 116
    [Homologação: Validado] => 117
    [Produção: Avaliar Produção] => 118
    [Fechado] => 5*/


//Filtro: CSOL1-TRAIKET_Metricas_Aberto
$c=$client->issue->all([       
   'project_id' => 'time-traiket',
'query_id' => 9189,  
'limit'=>4000
]);



$vetChamados[]=0;

$vetkanban['project_id']='time-traiket';
$vetkanban['id_filtro_redmine']='9189';


$vetkanban['qtd_chamados_Novo']=0;
$vetkanban['qtd_chamados_Aberto']=0;
$vetkanban['qtd_chamados_Desev_Andamento']=0;
$vetkanban['qtd_chamados_Desev_Resolvido']=0;
$vetkanban['qtd_chamados_PreHmlFazer']=0;
$vetkanban['qtd_chamados_PreHmlFazendo']=0;
$vetkanban['qtd_chamados_PreHmlFeito']=0;
$vetkanban['qtd_chamados_Hml_Andamento']=0;
$vetkanban['qtd_chamados_Hml_Validado']=0;
$vetkanban['qtd_chamados_AvaliarProducao']=0;
$vetkanban['qtd_chamados_rejeitados_IR']=0;
//$vetkanban['qtd_chamados_rejeitados']=0;
$vetkanban['qtd_tag_rejeicao']=0;
$vetkanban['qtd_tag_cancelamento']=0;
$vetkanban['qtd_tag_impedimento']=0;
$vetkanban['qtd_tag_Liberado']=0;
$vetkanban['qtd_total_chamados']=0;

$vetkanban['qtd_chamados_corretivos']=0;
$vetkanban['qtd_chamados_adaptativos']=0;
$vetkanban['qtd_chamados_evolutivos']=0;
$vetkanban['qtd_chamados_consultas']=0;

foreach ($c['issues'] as $key => $value) {    
    $vetkanban['qtd_total_chamados']++;
    $idChamado=$value['id'];            
    $issueId=1; //usar quando não se quer manter os dados por chamados. com isso ira sobrepor os dados no vetor e mantem-se apenas 
    
    if($value['tracker']['name']=='Corretiva')   
       $vetkanban['qtd_chamados_corretivos']++;
    if($value['tracker']['name']=='Adaptativa')   
       $vetkanban['qtd_chamados_adaptativos']++;
    if($value['tracker']['name']=='Evolutiva')   
       $vetkanban['qtd_chamados_evolutivos']++;
    if($value['tracker']['name']=='Consulta')   
       $vetkanban['qtd_chamados_consultas']++;         

    if($value['status']['id']=='120')   //[Backlog: Novo] => 120
        $vetkanban['qtd_chamados_Novo']++;
    if($value['status']['id']=='119')   //Backlog: Aberto 
        $vetkanban['qtd_chamados_Aberto']++;
    if($value['status']['id']=='114')   //[Desenvolvimento: Em Andamento] => 114
        $vetkanban['qtd_chamados_Desev_Andamento']++;
    if($value['status']['id']=='115')   //[Desenvolvimento: Resolvido] => 115
        $vetkanban['qtd_chamados_Desev_Resolvido']++;
    if($value['status']['id']=='131')   //Pré-HML: A Fazer 131
        $vetkanban['qtd_chamados_PreHmlFazer']++;
    if($value['status']['id']=='132')   //Pré-HML: Fazendo 132
        $vetkanban['qtd_chamados_PreHmlFazendo']++;
    if($value['status']['id']=='133')   //Pré-HML: Feito  133
        $vetkanban['qtd_chamados_PreHmlFeito']++;
    if($value['status']['id']=='116')   //[Homologação: Em Andamento] => 116
        $vetkanban['qtd_chamados_Hml_Andamento']++;
    if($value['status']['id']=='117')   //[Homologação: Validado] => 117
        $vetkanban['qtd_chamados_Hml_Validado']++;
    if($value['status']['id']=='118')   //[Produção: Avaliar Produção] => 118
        $vetkanban['qtd_chamados_AvaliarProducao']++;
            
    //Obter os chamados cancelados indevidamente    
    foreach ($value['custom_fields'] as $keyC => $valueC) {
        if($valueC['name']=='IR' && $valueC['value']=='Sim') {            
            $vetkanban['qtd_chamados_rejeitados_IR']++;
        }        
    }        
} //fim do foreach dos chamados
    
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
               $tag.=$valueD['new_value'];        
        }
    } //fim do fireach dos journals
    if (strpos($tag, 'Rejeitado')!==false) $vetkanban['qtd_tag_rejeicao']++;
    if (strpos($tag, 'Cancelamento')!==false) $vetkanban['qtd_tag_cancelamento']++;
    if (strpos($tag, 'Impedimento')!==false) $vetkanban['qtd_tag_impedimento']++;
    if (strpos($tag, 'Liberado (Em espera)')!==false) $vetkanban['qtd_tag_Liberado']++;
    
} //fim do foreach dos chamados

$vetkanban['wip']=$vetkanban['qtd_chamados_Desev_Andamento']+$vetkanban['qtd_chamados_Desev_Resolvido']+$vetkanban['qtd_chamados_PreHmlFazer']+$vetkanban['qtd_chamados_PreHmlFazendo']+$vetkanban['qtd_chamados_PreHmlFeito']+$vetkanban['qtd_chamados_Hml_Andamento']+$vetkanban['qtd_chamados_Hml_Validado']+$vetkanban['qtd_tag_rejeicao']+$vetkanban['qtd_tag_Liberado']+$vetkanban['qtd_tag_impedimento'];

$vetkanban['wip_perc']=round(($vetkanban['wip']*100)/$vetkanban['qtd_total_chamados'],2);

$dif_time_script=date_diff($vetkanban['tempo_execucao_script'], new DateTime(),true);
    $vetkanban['tempo_execucao_script']=$dif_time_script->format('%hhs %imin %sseg');

echo json_encode($vetkanban);
?>