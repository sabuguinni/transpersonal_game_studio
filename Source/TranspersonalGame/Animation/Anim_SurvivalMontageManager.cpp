#include "Anim_SurvivalMontageManager.h"
#include "Engine/Engine.h"
#include "Animation/AnimMontage.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"

DEFINE_LOG_CATEGORY_STATIC(LogSurvivalMontage, Log, All);

UAnim_SurvivalMontageManager::UAnim_SurvivalMontageManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // Tick a cada 100ms
    
    // Configurações padrão
    bIsPlayingMontage = false;
    CurrentMontageType = EAnim_SurvivalMontageType::None;
    MontageBlendOutTime = 0.25f;
    DefaultPlayRate = 1.0f;
    
    // Inicializar arrays
    QueuedMontages.Empty();
    ActiveMontageCallbacks.Empty();
}

void UAnim_SurvivalMontageManager::BeginPlay()
{
    Super::BeginPlay();
    
    // Obter referência ao character
    OwnerCharacter = Cast<ACharacter>(GetOwner());
    if (!OwnerCharacter)
    {
        UE_LOG(LogSurvivalMontage, Error, TEXT("SurvivalMontageManager deve ser anexado a um Character"));
        return;
    }
    
    // Obter componente de mesh
    SkeletalMeshComponent = OwnerCharacter->GetMesh();
    if (!SkeletalMeshComponent)
    {
        UE_LOG(LogSurvivalMontage, Error, TEXT("Character não tem SkeletalMeshComponent"));
        return;
    }
    
    // Obter AnimInstance
    AnimInstance = SkeletalMeshComponent->GetAnimInstance();
    if (!AnimInstance)
    {
        UE_LOG(LogSurvivalMontage, Warning, TEXT("SkeletalMeshComponent não tem AnimInstance"));
    }
    
    UE_LOG(LogSurvivalMontage, Log, TEXT("SurvivalMontageManager inicializado para %s"), 
           *OwnerCharacter->GetName());
}

void UAnim_SurvivalMontageManager::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    // Verificar se a montage actual terminou
    if (bIsPlayingMontage && AnimInstance)
    {
        if (!AnimInstance->IsAnyMontagePlaying())
        {
            OnMontageFinished();
        }
    }
    
    // Processar queue de montages
    ProcessMontageQueue();
}

bool UAnim_SurvivalMontageManager::PlaySurvivalMontage(EAnim_SurvivalMontageType MontageType, 
                                                      float PlayRate, bool bInterruptCurrent)
{
    if (!AnimInstance)
    {
        UE_LOG(LogSurvivalMontage, Warning, TEXT("Não é possível reproduzir montage - AnimInstance inválida"));
        return false;
    }
    
    // Verificar se já está a reproduzir uma montage
    if (bIsPlayingMontage && !bInterruptCurrent)
    {
        // Adicionar à queue
        FAnim_MontageQueueItem QueueItem;
        QueueItem.MontageType = MontageType;
        QueueItem.PlayRate = PlayRate;
        QueuedMontages.Add(QueueItem);
        
        UE_LOG(LogSurvivalMontage, Log, TEXT("Montage %s adicionada à queue"), 
               *UEnum::GetValueAsString(MontageType));
        return true;
    }
    
    // Obter a montage para o tipo especificado
    UAnimMontage* MontageToPlay = GetMontageForType(MontageType);
    if (!MontageToPlay)
    {
        UE_LOG(LogSurvivalMontage, Warning, TEXT("Montage não encontrada para tipo: %s"), 
               *UEnum::GetValueAsString(MontageType));
        return false;
    }
    
    // Parar montage actual se necessário
    if (bIsPlayingMontage && bInterruptCurrent)
    {
        StopCurrentMontage();
    }
    
    // Reproduzir a nova montage
    float MontageLength = AnimInstance->Montage_Play(MontageToPlay, PlayRate);
    if (MontageLength > 0.0f)
    {
        bIsPlayingMontage = true;
        CurrentMontageType = MontageType;
        CurrentMontage = MontageToPlay;
        
        UE_LOG(LogSurvivalMontage, Log, TEXT("Reproduzindo montage %s (duração: %.2fs)"), 
               *UEnum::GetValueAsString(MontageType), MontageLength);
        
        // Broadcast evento de início
        OnMontageStarted.Broadcast(MontageType, MontageToPlay);
        
        return true;
    }
    
    UE_LOG(LogSurvivalMontage, Error, TEXT("Falha ao reproduzir montage %s"), 
           *UEnum::GetValueAsString(MontageType));
    return false;
}

void UAnim_SurvivalMontageManager::StopCurrentMontage()
{
    if (!bIsPlayingMontage || !AnimInstance)
        return;
    
    AnimInstance->Montage_Stop(MontageBlendOutTime, CurrentMontage);
    
    UE_LOG(LogSurvivalMontage, Log, TEXT("Parando montage %s"), 
           *UEnum::GetValueAsString(CurrentMontageType));
    
    OnMontageFinished();
}

void UAnim_SurvivalMontageManager::ClearMontageQueue()
{
    QueuedMontages.Empty();
    UE_LOG(LogSurvivalMontage, Log, TEXT("Queue de montages limpa"));
}

bool UAnim_SurvivalMontageManager::IsPlayingMontageOfType(EAnim_SurvivalMontageType MontageType) const
{
    return bIsPlayingMontage && CurrentMontageType == MontageType;
}

float UAnim_SurvivalMontageManager::GetCurrentMontagePosition() const
{
    if (!bIsPlayingMontage || !AnimInstance || !CurrentMontage)
        return 0.0f;
    
    return AnimInstance->Montage_GetPosition(CurrentMontage);
}

float UAnim_SurvivalMontageManager::GetCurrentMontageLength() const
{
    if (!CurrentMontage)
        return 0.0f;
    
    return CurrentMontage->GetPlayLength();
}

void UAnim_SurvivalMontageManager::SetMontageForType(EAnim_SurvivalMontageType MontageType, 
                                                    UAnimMontage* Montage)
{
    if (!Montage)
    {
        UE_LOG(LogSurvivalMontage, Warning, TEXT("Tentativa de definir montage nula para tipo %s"), 
               *UEnum::GetValueAsString(MontageType));
        return;
    }
    
    SurvivalMontages.Add(MontageType, Montage);
    UE_LOG(LogSurvivalMontage, Log, TEXT("Montage %s definida para tipo %s"), 
           *Montage->GetName(), *UEnum::GetValueAsString(MontageType));
}

UAnimMontage* UAnim_SurvivalMontageManager::GetMontageForType(EAnim_SurvivalMontageType MontageType) const
{
    if (const TObjectPtr<UAnimMontage>* FoundMontage = SurvivalMontages.Find(MontageType))
    {
        return *FoundMontage;
    }
    
    return nullptr;
}

void UAnim_SurvivalMontageManager::OnMontageFinished()
{
    if (!bIsPlayingMontage)
        return;
    
    EAnim_SurvivalMontageType FinishedType = CurrentMontageType;
    UAnimMontage* FinishedMontage = CurrentMontage;
    
    // Reset estado
    bIsPlayingMontage = false;
    CurrentMontageType = EAnim_SurvivalMontageType::None;
    CurrentMontage = nullptr;
    
    UE_LOG(LogSurvivalMontage, Log, TEXT("Montage %s terminou"), 
           *UEnum::GetValueAsString(FinishedType));
    
    // Broadcast evento de fim
    OnMontageFinished.Broadcast(FinishedType, FinishedMontage);
}

void UAnim_SurvivalMontageManager::ProcessMontageQueue()
{
    // Se não estamos a reproduzir nada e temos items na queue
    if (!bIsPlayingMontage && QueuedMontages.Num() > 0)
    {
        FAnim_MontageQueueItem NextItem = QueuedMontages[0];
        QueuedMontages.RemoveAt(0);
        
        PlaySurvivalMontage(NextItem.MontageType, NextItem.PlayRate, false);
    }
}

// Funções de conveniência para tipos específicos de montage
bool UAnim_SurvivalMontageManager::PlayGatheringMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Gathering, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlayCraftingMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Crafting, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlayHuntingMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Hunting, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlayBuildingMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Building, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlayEatingMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Eating, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlayDrinkingMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Drinking, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlaySleepingMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Sleeping, PlayRate);
}

bool UAnim_SurvivalMontageManager::PlayInjuredMontage(float PlayRate)
{
    return PlaySurvivalMontage(EAnim_SurvivalMontageType::Injured, PlayRate);
}