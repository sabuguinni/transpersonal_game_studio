#include "Anim_SurvivalMontageController.h"
#include "Animation/AnimInstance.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Character.h"
#include "Engine/World.h"
#include "TimerManager.h"

UAnim_SurvivalMontageController::UAnim_SurvivalMontageController()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    TargetMeshComponent = nullptr;
    CurrentMontage = nullptr;
    CurrentAction = EAnim_SurvivalAction::None;
    ActionTimer = 0.0f;
    MaxActionDuration = 10.0f;
}

void UAnim_SurvivalMontageController::BeginPlay()
{
    Super::BeginPlay();
    
    // Tentar encontrar automaticamente o SkeletalMeshComponent
    if (!TargetMeshComponent)
    {
        if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
        {
            TargetMeshComponent = OwnerCharacter->GetMesh();
        }
        else
        {
            TargetMeshComponent = GetOwner()->FindComponentByClass<USkeletalMeshComponent>();
        }
    }
    
    InitializeDefaultMontages();
}

void UAnim_SurvivalMontageController::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                   FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    UpdateActionTimer(DeltaTime);
}

bool UAnim_SurvivalMontageController::PlaySurvivalAction(EAnim_SurvivalAction Action, float Duration)
{
    if (!TargetMeshComponent || !TargetMeshComponent->GetAnimInstance())
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalMontageController: No valid mesh component or anim instance"));
        return false;
    }
    
    FAnim_SurvivalMontageData* MontageData = FindMontageData(Action);
    if (!MontageData || !MontageData->Montage)
    {
        UE_LOG(LogTemp, Warning, TEXT("SurvivalMontageController: No montage found for action %d"), (int32)Action);
        return false;
    }
    
    // Parar montage atual se estiver reproduzindo
    if (CurrentMontage)
    {
        TargetMeshComponent->GetAnimInstance()->Montage_Stop(MontageData->BlendOutTime, CurrentMontage);
    }
    
    // Reproduzir novo montage
    float MontageLength = TargetMeshComponent->GetAnimInstance()->Montage_Play(
        MontageData->Montage,
        MontageData->PlayRate,
        EMontagePlayReturnType::MontageLength,
        0.0f,
        true
    );
    
    if (MontageLength > 0.0f)
    {
        CurrentMontage = MontageData->Montage;
        CurrentAction = Action;
        
        // Configurar timer para duração da ação
        if (Duration > 0.0f)
        {
            ActionTimer = Duration;
        }
        else if (MontageData->bLooping)
        {
            ActionTimer = MaxActionDuration;
        }
        else
        {
            ActionTimer = MontageLength / MontageData->PlayRate;
        }
        
        // Bind callback para quando montage terminar
        if (UAnimInstance* AnimInstance = TargetMeshComponent->GetAnimInstance())
        {
            AnimInstance->OnMontageEnded.AddDynamic(this, &UAnim_SurvivalMontageController::OnMontageEnded);
        }
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalMontageController: Playing action %d for %.2f seconds"), 
               (int32)Action, ActionTimer);
        
        return true;
    }
    
    return false;
}

void UAnim_SurvivalMontageController::StopCurrentAction()
{
    if (CurrentMontage && TargetMeshComponent && TargetMeshComponent->GetAnimInstance())
    {
        FAnim_SurvivalMontageData* MontageData = FindMontageData(CurrentAction);
        float BlendOutTime = MontageData ? MontageData->BlendOutTime : 0.25f;
        
        TargetMeshComponent->GetAnimInstance()->Montage_Stop(BlendOutTime, CurrentMontage);
        
        CurrentMontage = nullptr;
        CurrentAction = EAnim_SurvivalAction::None;
        ActionTimer = 0.0f;
        
        UE_LOG(LogTemp, Log, TEXT("SurvivalMontageController: Stopped current action"));
    }
}

bool UAnim_SurvivalMontageController::IsPlayingAction(EAnim_SurvivalAction Action) const
{
    return CurrentAction == Action && CurrentMontage != nullptr;
}

void UAnim_SurvivalMontageController::SetTargetMeshComponent(USkeletalMeshComponent* MeshComponent)
{
    TargetMeshComponent = MeshComponent;
}

void UAnim_SurvivalMontageController::AddSurvivalMontage(EAnim_SurvivalAction Action, UAnimMontage* Montage, 
                                                        float PlayRate, bool bLooping)
{
    if (!Montage)
    {
        return;
    }
    
    // Procurar se já existe entrada para esta ação
    FAnim_SurvivalMontageData* ExistingData = FindMontageData(Action);
    if (ExistingData)
    {
        ExistingData->Montage = Montage;
        ExistingData->PlayRate = PlayRate;
        ExistingData->bLooping = bLooping;
    }
    else
    {
        // Criar nova entrada
        FAnim_SurvivalMontageData NewData;
        NewData.Action = Action;
        NewData.Montage = Montage;
        NewData.PlayRate = PlayRate;
        NewData.bLooping = bLooping;
        
        SurvivalMontages.Add(NewData);
    }
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalMontageController: Added montage for action %d"), (int32)Action);
}

void UAnim_SurvivalMontageController::RemoveSurvivalMontage(EAnim_SurvivalAction Action)
{
    for (int32 i = SurvivalMontages.Num() - 1; i >= 0; i--)
    {
        if (SurvivalMontages[i].Action == Action)
        {
            SurvivalMontages.RemoveAt(i);
            UE_LOG(LogTemp, Log, TEXT("SurvivalMontageController: Removed montage for action %d"), (int32)Action);
            break;
        }
    }
}

void UAnim_SurvivalMontageController::SetActionBlendTimes(EAnim_SurvivalAction Action, float BlendIn, float BlendOut)
{
    FAnim_SurvivalMontageData* MontageData = FindMontageData(Action);
    if (MontageData)
    {
        MontageData->BlendInTime = BlendIn;
        MontageData->BlendOutTime = BlendOut;
    }
}

FAnim_SurvivalMontageData* UAnim_SurvivalMontageController::FindMontageData(EAnim_SurvivalAction Action)
{
    for (FAnim_SurvivalMontageData& Data : SurvivalMontages)
    {
        if (Data.Action == Action)
        {
            return &Data;
        }
    }
    return nullptr;
}

void UAnim_SurvivalMontageController::OnMontageEnded(UAnimMontage* Montage, bool bInterrupted)
{
    if (Montage == CurrentMontage)
    {
        UE_LOG(LogTemp, Log, TEXT("SurvivalMontageController: Montage ended %s"), 
               bInterrupted ? TEXT("(interrupted)") : TEXT("(completed)"));
        
        // Se o montage está configurado para loop e não foi interrompido, reiniciar
        FAnim_SurvivalMontageData* MontageData = FindMontageData(CurrentAction);
        if (MontageData && MontageData->bLooping && !bInterrupted && ActionTimer > 0.0f)
        {
            // Reiniciar montage
            TargetMeshComponent->GetAnimInstance()->Montage_Play(
                MontageData->Montage,
                MontageData->PlayRate,
                EMontagePlayReturnType::MontageLength,
                0.0f,
                true
            );
        }
        else
        {
            CurrentMontage = nullptr;
            CurrentAction = EAnim_SurvivalAction::None;
            ActionTimer = 0.0f;
        }
    }
}

void UAnim_SurvivalMontageController::UpdateActionTimer(float DeltaTime)
{
    if (ActionTimer > 0.0f)
    {
        ActionTimer -= DeltaTime;
        
        if (ActionTimer <= 0.0f)
        {
            // Timer expirou, parar ação
            StopCurrentAction();
        }
    }
}

void UAnim_SurvivalMontageController::InitializeDefaultMontages()
{
    // Inicializar com montages padrão se disponíveis
    // Nota: Os montages reais serão configurados via Blueprint ou código
    
    UE_LOG(LogTemp, Log, TEXT("SurvivalMontageController: Initialized with %d survival montages"), 
           SurvivalMontages.Num());
}