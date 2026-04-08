#include "MetaSoundManager.h"
#include "Engine/World.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "AudioComponent.h"
#include "MetasoundSource.h"
#include "AudioParameterControllerInterface.h"

DEFINE_LOG_CATEGORY(LogMetaSoundManager);

UMetaSoundManager::UMetaSoundManager()
{
    PrimaryComponentTick.bCanEverTick = true;
    bWantsInitializeComponent = true;

    MaxConcurrentMetaSounds = 32;
    MaxCPUPercentage = 15.0f;
    CurrentCPUUsage = 0.0f;
    ActiveInstanceCount = 0;
}

void UMetaSoundManager::BeginPlay()
{
    Super::BeginPlay();

    // Start performance monitoring
    GetWorld()->GetTimerManager().SetTimer(
        PerformanceCheckTimer,
        this,
        &UMetaSoundManager::ManagePerformance,
        1.0f, // Check every second
        true
    );

    UE_LOG(LogMetaSoundManager, Log, TEXT("MetaSoundManager initialized with max %d concurrent sounds"), MaxConcurrentMetaSounds);
}

void UMetaSoundManager::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    // Update parameter interpolations
    UpdateParameterInterpolations(DeltaTime);

    // Clean up inactive instances periodically
    static float CleanupTimer = 0.0f;
    CleanupTimer += DeltaTime;
    if (CleanupTimer >= 5.0f) // Every 5 seconds
    {
        CleanupInactiveInstances();
        CleanupTimer = 0.0f;
    }
}

bool UMetaSoundManager::CreateMetaSoundInstance(EMetaSoundType Type, const FString& InstanceName, UMetaSoundSource* MetaSoundAsset)
{
    if (!MetaSoundAsset)
    {
        UE_LOG(LogMetaSoundManager, Warning, TEXT("Cannot create MetaSound instance '%s': MetaSoundAsset is null"), *InstanceName);
        return false;
    }

    if (MetaSoundInstances.Contains(InstanceName))
    {
        UE_LOG(LogMetaSoundManager, Warning, TEXT("MetaSound instance '%s' already exists"), *InstanceName);
        return false;
    }

    // Create audio component
    UAudioComponent* AudioComp = NewObject<UAudioComponent>(GetOwner());
    if (!AudioComp)
    {
        UE_LOG(LogMetaSoundManager, Error, TEXT("Failed to create AudioComponent for instance '%s'"), *InstanceName);
        return false;
    }

    // Configure audio component
    AudioComp->SetSound(MetaSoundAsset);
    AudioComp->bAutoActivate = false;
    AudioComp->AttachToComponent(GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepWorldTransform);

    // Create MetaSound instance
    FMetaSoundInstance NewInstance;
    NewInstance.MetaSoundAsset = MetaSoundAsset;
    NewInstance.AudioComponent = AudioComp;
    NewInstance.bIsActive = false;
    NewInstance.Priority = 1.0f;

    // Store instance
    MetaSoundInstances.Add(InstanceName, NewInstance);

    // Add to type-based organization
    if (!InstancesByType.Contains(Type))
    {
        InstancesByType.Add(Type, TArray<FString>());
    }
    InstancesByType[Type].Add(InstanceName);

    UE_LOG(LogMetaSoundManager, Log, TEXT("Created MetaSound instance '%s' of type %d"), *InstanceName, (int32)Type);
    return true;
}

bool UMetaSoundManager::PlayMetaSoundInstance(const FString& InstanceName, bool bLoop)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    if (Instance.bIsActive)
    {
        UE_LOG(LogMetaSoundManager, Warning, TEXT("MetaSound instance '%s' is already playing"), *InstanceName);
        return false;
    }

    // Check performance limits
    if (ActiveInstanceCount >= MaxConcurrentMetaSounds)
    {
        UE_LOG(LogMetaSoundManager, Warning, TEXT("Cannot play '%s': Max concurrent MetaSounds reached (%d)"), 
               *InstanceName, MaxConcurrentMetaSounds);
        return false;
    }

    // Configure and play
    Instance.AudioComponent->bLooping = bLoop;
    Instance.AudioComponent->Play();
    Instance.bIsActive = true;
    ActiveInstanceCount++;

    UE_LOG(LogMetaSoundManager, Log, TEXT("Playing MetaSound instance '%s' (Loop: %s)"), 
           *InstanceName, bLoop ? TEXT("Yes") : TEXT("No"));
    return true;
}

bool UMetaSoundManager::StopMetaSoundInstance(const FString& InstanceName, float FadeOutTime)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    if (!Instance.bIsActive)
    {
        UE_LOG(LogMetaSoundManager, Warning, TEXT("MetaSound instance '%s' is not playing"), *InstanceName);
        return false;
    }

    if (FadeOutTime > 0.0f)
    {
        Instance.AudioComponent->FadeOut(FadeOutTime, 0.0f);
    }
    else
    {
        Instance.AudioComponent->Stop();
    }

    Instance.bIsActive = false;
    ActiveInstanceCount = FMath::Max(0, ActiveInstanceCount - 1);

    UE_LOG(LogMetaSoundManager, Log, TEXT("Stopping MetaSound instance '%s' (FadeOut: %.2fs)"), 
           *InstanceName, FadeOutTime);
    return true;
}

bool UMetaSoundManager::SetFloatParameter(const FString& InstanceName, const FString& ParameterName, float Value)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    // Update parameter in AudioComponent
    Instance.AudioComponent->SetFloatParameter(FName(*ParameterName), Value);
    
    // Store parameter value
    Instance.FloatParameters.Add(ParameterName, Value);

    UE_LOG(LogMetaSoundManager, VeryVerbose, TEXT("Set float parameter '%s' = %.3f for instance '%s'"), 
           *ParameterName, Value, *InstanceName);
    return true;
}

bool UMetaSoundManager::SetBoolParameter(const FString& InstanceName, const FString& ParameterName, bool Value)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    // Update parameter in AudioComponent
    Instance.AudioComponent->SetBoolParameter(FName(*ParameterName), Value);
    
    // Store parameter value
    Instance.BoolParameters.Add(ParameterName, Value);

    UE_LOG(LogMetaSoundManager, VeryVerbose, TEXT("Set bool parameter '%s' = %s for instance '%s'"), 
           *ParameterName, Value ? TEXT("true") : TEXT("false"), *InstanceName);
    return true;
}

bool UMetaSoundManager::SetIntParameter(const FString& InstanceName, const FString& ParameterName, int32 Value)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    // Update parameter in AudioComponent
    Instance.AudioComponent->SetIntParameter(FName(*ParameterName), Value);
    
    // Store parameter value
    Instance.IntParameters.Add(ParameterName, Value);

    UE_LOG(LogMetaSoundManager, VeryVerbose, TEXT("Set int parameter '%s' = %d for instance '%s'"), 
           *ParameterName, Value, *InstanceName);
    return true;
}

bool UMetaSoundManager::InterpolateFloatParameter(const FString& InstanceName, const FString& ParameterName, 
                                                 float TargetValue, float InterpolationTime)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    const FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    // Get current parameter value
    float CurrentValue = 0.0f;
    if (const float* ExistingValue = Instance.FloatParameters.Find(ParameterName))
    {
        CurrentValue = *ExistingValue;
    }

    // Create interpolation data
    FFloatParameterInterpolation Interpolation;
    Interpolation.StartValue = CurrentValue;
    Interpolation.TargetValue = TargetValue;
    Interpolation.CurrentTime = 0.0f;
    Interpolation.Duration = FMath::Max(0.01f, InterpolationTime);
    Interpolation.InstanceName = InstanceName;
    Interpolation.ParameterName = ParameterName;

    // Store interpolation
    if (!ActiveInterpolations.Contains(InstanceName))
    {
        ActiveInterpolations.Add(InstanceName, TMap<FString, FFloatParameterInterpolation>());
    }
    ActiveInterpolations[InstanceName].Add(ParameterName, Interpolation);

    UE_LOG(LogMetaSoundManager, Log, TEXT("Started parameter interpolation '%s' from %.3f to %.3f over %.2fs for instance '%s'"), 
           *ParameterName, CurrentValue, TargetValue, InterpolationTime, *InstanceName);
    return true;
}

bool UMetaSoundManager::TriggerMetaSoundEvent(const FString& InstanceName, const FString& TriggerName)
{
    if (!ValidateMetaSoundInstance(InstanceName))
    {
        return false;
    }

    FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    
    // Trigger event in AudioComponent
    Instance.AudioComponent->SetTriggerParameter(FName(*TriggerName));

    UE_LOG(LogMetaSoundManager, Log, TEXT("Triggered event '%s' for instance '%s'"), 
           *TriggerName, *InstanceName);
    return true;
}

void UMetaSoundManager::SetGlobalFloatParameter(const FString& ParameterName, float Value)
{
    for (auto& InstancePair : MetaSoundInstances)
    {
        SetFloatParameter(InstancePair.Key, ParameterName, Value);
    }

    UE_LOG(LogMetaSoundManager, Log, TEXT("Set global float parameter '%s' = %.3f for all instances"), 
           *ParameterName, Value);
}

void UMetaSoundManager::StopAllMetaSoundsOfType(EMetaSoundType Type, float FadeOutTime)
{
    if (!InstancesByType.Contains(Type))
    {
        return;
    }

    const TArray<FString>& Instances = InstancesByType[Type];
    for (const FString& InstanceName : Instances)
    {
        StopMetaSoundInstance(InstanceName, FadeOutTime);
    }

    UE_LOG(LogMetaSoundManager, Log, TEXT("Stopped all MetaSound instances of type %d"), (int32)Type);
}

bool UMetaSoundManager::IsMetaSoundInstanceActive(const FString& InstanceName) const
{
    if (const FMetaSoundInstance* Instance = MetaSoundInstances.Find(InstanceName))
    {
        return Instance->bIsActive;
    }
    return false;
}

float UMetaSoundManager::GetFloatParameter(const FString& InstanceName, const FString& ParameterName) const
{
    if (const FMetaSoundInstance* Instance = MetaSoundInstances.Find(InstanceName))
    {
        if (const float* Value = Instance->FloatParameters.Find(ParameterName))
        {
            return *Value;
        }
    }
    return 0.0f;
}

TArray<FString> UMetaSoundManager::GetActiveMetaSoundInstances() const
{
    TArray<FString> ActiveInstances;
    for (const auto& InstancePair : MetaSoundInstances)
    {
        if (InstancePair.Value.bIsActive)
        {
            ActiveInstances.Add(InstancePair.Key);
        }
    }
    return ActiveInstances;
}

void UMetaSoundManager::SetMaxConcurrentMetaSounds(int32 MaxCount)
{
    MaxConcurrentMetaSounds = FMath::Max(1, MaxCount);
    UE_LOG(LogMetaSoundManager, Log, TEXT("Set max concurrent MetaSounds to %d"), MaxConcurrentMetaSounds);
}

void UMetaSoundManager::SetCPUBudget(float MaxCPUPercentage)
{
    MaxCPUPercentage = FMath::Clamp(MaxCPUPercentage, 1.0f, 50.0f);
    UE_LOG(LogMetaSoundManager, Log, TEXT("Set CPU budget to %.1f%%"), MaxCPUPercentage);
}

void UMetaSoundManager::UpdateParameterInterpolations(float DeltaTime)
{
    TArray<FString> CompletedInterpolations;

    for (auto& InstancePair : ActiveInterpolations)
    {
        const FString& InstanceName = InstancePair.Key;
        auto& ParameterInterpolations = InstancePair.Value;

        TArray<FString> CompletedParameters;

        for (auto& ParamPair : ParameterInterpolations)
        {
            const FString& ParameterName = ParamPair.Key;
            FFloatParameterInterpolation& Interpolation = ParamPair.Value;

            Interpolation.CurrentTime += DeltaTime;
            float Alpha = FMath::Clamp(Interpolation.CurrentTime / Interpolation.Duration, 0.0f, 1.0f);
            
            // Smooth interpolation using ease-in-out
            Alpha = FMath::SmoothStep(0.0f, 1.0f, Alpha);
            
            float CurrentValue = FMath::Lerp(Interpolation.StartValue, Interpolation.TargetValue, Alpha);
            SetFloatParameter(InstanceName, ParameterName, CurrentValue);

            if (Interpolation.CurrentTime >= Interpolation.Duration)
            {
                CompletedParameters.Add(ParameterName);
            }
        }

        // Remove completed interpolations
        for (const FString& CompletedParam : CompletedParameters)
        {
            ParameterInterpolations.Remove(CompletedParam);
        }

        if (ParameterInterpolations.Num() == 0)
        {
            CompletedInterpolations.Add(InstanceName);
        }
    }

    // Remove instances with no active interpolations
    for (const FString& CompletedInstance : CompletedInterpolations)
    {
        ActiveInterpolations.Remove(CompletedInstance);
    }
}

void UMetaSoundManager::ManagePerformance()
{
    // Update active instance count
    ActiveInstanceCount = 0;
    for (const auto& InstancePair : MetaSoundInstances)
    {
        if (InstancePair.Value.bIsActive)
        {
            ActiveInstanceCount++;
        }
    }

    // Simple CPU usage estimation (this would be more sophisticated in a real implementation)
    CurrentCPUUsage = (float)ActiveInstanceCount / (float)MaxConcurrentMetaSounds * 100.0f;

    // If we're over budget, stop lowest priority instances
    if (CurrentCPUUsage > MaxCPUPercentage)
    {
        // Find lowest priority active instance
        FString LowestPriorityInstance;
        float LowestPriority = FLT_MAX;

        for (const auto& InstancePair : MetaSoundInstances)
        {
            if (InstancePair.Value.bIsActive && InstancePair.Value.Priority < LowestPriority)
            {
                LowestPriority = InstancePair.Value.Priority;
                LowestPriorityInstance = InstancePair.Key;
            }
        }

        if (!LowestPriorityInstance.IsEmpty())
        {
            StopMetaSoundInstance(LowestPriorityInstance, 0.5f);
            UE_LOG(LogMetaSoundManager, Warning, TEXT("Stopped low priority instance '%s' due to CPU budget"), 
                   *LowestPriorityInstance);
        }
    }
}

bool UMetaSoundManager::ValidateMetaSoundInstance(const FString& InstanceName) const
{
    if (!MetaSoundInstances.Contains(InstanceName))
    {
        UE_LOG(LogMetaSoundManager, Warning, TEXT("MetaSound instance '%s' does not exist"), *InstanceName);
        return false;
    }

    const FMetaSoundInstance& Instance = MetaSoundInstances[InstanceName];
    if (!Instance.AudioComponent)
    {
        UE_LOG(LogMetaSoundManager, Error, TEXT("MetaSound instance '%s' has null AudioComponent"), *InstanceName);
        return false;
    }

    return true;
}

void UMetaSoundManager::CleanupInactiveInstances()
{
    TArray<FString> InstancesToRemove;

    for (const auto& InstancePair : MetaSoundInstances)
    {
        const FMetaSoundInstance& Instance = InstancePair.Value;
        
        // Check if audio component is valid and if sound has finished playing
        if (Instance.AudioComponent && !Instance.AudioComponent->IsPlaying() && Instance.bIsActive)
        {
            // Mark as inactive
            const_cast<FMetaSoundInstance&>(Instance).bIsActive = false;
            ActiveInstanceCount = FMath::Max(0, ActiveInstanceCount - 1);
        }
    }

    UE_LOG(LogMetaSoundManager, VeryVerbose, TEXT("Cleaned up inactive MetaSound instances"));
}