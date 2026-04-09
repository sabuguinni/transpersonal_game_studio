#include "NarrativeVFXSystem.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"

DEFINE_LOG_CATEGORY(LogNarrativeVFX);

UNarrativeVFXSystem::UNarrativeVFXSystem()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = 0.1f; // 10Hz update rate
    
    // Initialize default settings
    NarrativeSettings.MaxActiveNarrativeVFX = 20;
    NarrativeSettings.EmotionalIntensityMultiplier = 1.0f;
    NarrativeSettings.bEnableEmotionalVFX = true;
    NarrativeSettings.bEnableStoryMomentVFX = true;
    NarrativeSettings.bEnableCharacterEmotionVFX = true;
    
    CurrentEmotionalState = ENarrativeEmotionalState::Neutral;
    CurrentStoryBeat = ENarrativeStoryBeat::Exploration;
    EmotionalIntensity = 0.5f;
    StoryProgressionValue = 0.0f;
    
    bIsSystemActive = false;
    bEmotionalTransitionInProgress = false;
}

void UNarrativeVFXSystem::BeginPlay()
{
    Super::BeginPlay();
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Narrative VFX System initialized"));
    InitializeNarrativeVFXSystem();
    
    // Cache player reference
    if (UWorld* World = GetWorld())
    {
        PlayerController = World->GetFirstPlayerController();
    }
}

void UNarrativeVFXSystem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsSystemActive) return;
    
    UpdateEmotionalTransitions(DeltaTime);
    UpdateActiveNarrativeVFX(DeltaTime);
    UpdateStoryProgression(DeltaTime);
    
    // Performance monitoring
    if (ActiveNarrativeVFX.Num() > NarrativeSettings.MaxActiveNarrativeVFX)
    {
        CullExcessiveNarrativeVFX();
    }
}

void UNarrativeVFXSystem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    CleanupNarrativeVFXSystem();
    Super::EndPlay(EndPlayReason);
}

void UNarrativeVFXSystem::InitializeNarrativeVFXSystem()
{
    bIsSystemActive = true;
    
    // Initialize emotional VFX pools
    InitializeEmotionalVFXPools();
    
    // Setup story beat tracking
    InitializeStoryBeatTracking();
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Narrative VFX System fully initialized with %d emotional states"), 
           static_cast<int32>(ENarrativeEmotionalState::MAX));
}

void UNarrativeVFXSystem::TriggerStoryMomentVFX(ENarrativeStoryBeat StoryBeat, FVector Location, float Intensity)
{
    if (!bIsSystemActive || !NarrativeSettings.bEnableStoryMomentVFX)
    {
        return;
    }
    
    // Find appropriate VFX for this story beat
    FNarrativeVFXData* VFXData = FindVFXForStoryBeat(StoryBeat);
    if (!VFXData)
    {
        UE_LOG(LogNarrativeVFX, Warning, TEXT("No VFX data found for story beat: %d"), static_cast<int32>(StoryBeat));
        return;
    }
    
    // Create VFX instance
    FNarrativeVFXInstance NewInstance;
    NewInstance.InstanceID = GenerateVFXInstanceID();
    NewInstance.VFXData = *VFXData;
    NewInstance.WorldLocation = Location;
    NewInstance.Intensity = FMath::Clamp(Intensity, 0.0f, 1.0f);
    NewInstance.StartTime = GetWorld()->GetTimeSeconds();
    NewInstance.bIsActive = true;
    NewInstance.EmotionalState = CurrentEmotionalState;
    NewInstance.StoryBeat = StoryBeat;
    
    // Spawn Niagara component
    if (SpawnNarrativeVFXInstance(NewInstance))
    {
        ActiveNarrativeVFX.Add(NewInstance);
        CurrentStoryBeat = StoryBeat;
        
        UE_LOG(LogNarrativeVFX, Log, TEXT("Triggered story moment VFX: %s at intensity %.2f"), 
               *VFXData->EffectName, Intensity);
    }
}

void UNarrativeVFXSystem::SetEmotionalState(ENarrativeEmotionalState NewState, float TransitionTime)
{
    if (CurrentEmotionalState == NewState) return;
    
    ENarrativeEmotionalState PreviousState = CurrentEmotionalState;
    CurrentEmotionalState = NewState;
    
    if (TransitionTime > 0.0f)
    {
        StartEmotionalTransition(PreviousState, NewState, TransitionTime);
    }
    else
    {
        // Instant transition
        ApplyEmotionalStateVFX(NewState, 1.0f);
    }
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Emotional state changed from %d to %d"), 
           static_cast<int32>(PreviousState), static_cast<int32>(NewState));
}

void UNarrativeVFXSystem::SetEmotionalIntensity(float NewIntensity, float TransitionTime)
{
    float ClampedIntensity = FMath::Clamp(NewIntensity, 0.0f, 1.0f);
    
    if (FMath::IsNearlyEqual(EmotionalIntensity, ClampedIntensity, 0.01f)) return;
    
    if (TransitionTime > 0.0f)
    {
        StartIntensityTransition(ClampedIntensity, TransitionTime);
    }
    else
    {
        EmotionalIntensity = ClampedIntensity;
        UpdateEmotionalVFXIntensity();
    }
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Emotional intensity set to %.2f"), ClampedIntensity);
}

void UNarrativeVFXSystem::TriggerCharacterEmotionVFX(AActor* Character, ENarrativeCharacterEmotion Emotion, float Duration)
{
    if (!Character || !NarrativeSettings.bEnableCharacterEmotionVFX) return;
    
    FCharacterEmotionVFX EmotionVFX;
    EmotionVFX.Character = Character;
    EmotionVFX.Emotion = Emotion;
    EmotionVFX.StartTime = GetWorld()->GetTimeSeconds();
    EmotionVFX.Duration = Duration;
    EmotionVFX.bIsActive = true;
    
    // Find attachment point on character
    FVector AttachLocation = Character->GetActorLocation();
    if (UStaticMeshComponent* MeshComp = Character->FindComponentByClass<UStaticMeshComponent>())
    {
        AttachLocation = MeshComp->GetComponentLocation();
        AttachLocation.Z += 100.0f; // Offset above character
    }
    
    // Spawn emotion VFX
    FNarrativeVFXData* EmotionVFXData = FindVFXForCharacterEmotion(Emotion);
    if (EmotionVFXData)
    {
        FNarrativeVFXInstance EmotionInstance;
        EmotionInstance.InstanceID = GenerateVFXInstanceID();
        EmotionInstance.VFXData = *EmotionVFXData;
        EmotionInstance.WorldLocation = AttachLocation;
        EmotionInstance.Intensity = 0.8f;
        EmotionInstance.StartTime = EmotionVFX.StartTime;
        EmotionInstance.bIsActive = true;
        EmotionInstance.AttachedActor = Character;
        
        if (SpawnNarrativeVFXInstance(EmotionInstance))
        {
            EmotionVFX.VFXInstanceID = EmotionInstance.InstanceID;
            ActiveCharacterEmotions.Add(EmotionVFX);
            
            UE_LOG(LogNarrativeVFX, Log, TEXT("Character emotion VFX triggered: %s for %s"), 
                   *EmotionVFXData->EffectName, *Character->GetName());
        }
    }
}

void UNarrativeVFXSystem::UpdateStoryProgression(float ProgressValue)
{
    StoryProgressionValue = FMath::Clamp(ProgressValue, 0.0f, 1.0f);
    
    // Update environmental VFX based on story progression
    UpdateEnvironmentalStoryVFX();
    
    UE_LOG(LogNarrativeVFX, VeryVerbose, TEXT("Story progression updated: %.2f"), StoryProgressionValue);
}

void UNarrativeVFXSystem::TriggerEnvironmentalStoryVFX(FVector Location, float Radius, ENarrativeStoryBeat StoryContext)
{
    if (!NarrativeSettings.bEnableStoryMomentVFX) return;
    
    // Create environmental story effect
    FEnvironmentalStoryVFX EnvironmentalVFX;
    EnvironmentalVFX.Location = Location;
    EnvironmentalVFX.Radius = Radius;
    EnvironmentalVFX.StoryContext = StoryContext;
    EnvironmentalVFX.StartTime = GetWorld()->GetTimeSeconds();
    EnvironmentalVFX.bIsActive = true;
    
    // Spawn multiple VFX instances within the radius
    int32 VFXCount = FMath::RandRange(3, 8);
    for (int32 i = 0; i < VFXCount; i++)
    {
        FVector RandomOffset = FMath::VRand() * FMath::RandRange(0.0f, Radius);
        RandomOffset.Z = FMath::Abs(RandomOffset.Z) * 0.5f; // Keep mostly horizontal
        
        FVector SpawnLocation = Location + RandomOffset;
        TriggerStoryMomentVFX(StoryContext, SpawnLocation, 0.6f);
    }
    
    ActiveEnvironmentalStoryVFX.Add(EnvironmentalVFX);
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Environmental story VFX triggered at location: %s"), 
           *Location.ToString());
}

void UNarrativeVFXSystem::SetNarrativeVFXSettings(const FNarrativeVFXSettings& Settings)
{
    NarrativeSettings = Settings;
    
    // Apply new settings to active VFX
    UpdateActiveNarrativeVFXSettings();
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Narrative VFX settings updated"));
}

void UNarrativeVFXSystem::EnableEmotionalVFX(bool bEnable)
{
    NarrativeSettings.bEnableEmotionalVFX = bEnable;
    
    if (!bEnable)
    {
        // Cleanup active emotional VFX
        CleanupEmotionalVFX();
    }
}

void UNarrativeVFXSystem::EnableStoryMomentVFX(bool bEnable)
{
    NarrativeSettings.bEnableStoryMomentVFX = bEnable;
    
    if (!bEnable)
    {
        // Cleanup active story moment VFX
        CleanupStoryMomentVFX();
    }
}

void UNarrativeVFXSystem::EnableCharacterEmotionVFX(bool bEnable)
{
    NarrativeSettings.bEnableCharacterEmotionVFX = bEnable;
    
    if (!bEnable)
    {
        // Cleanup active character emotion VFX
        CleanupCharacterEmotionVFX();
    }
}

TArray<FNarrativeVFXInstance> UNarrativeVFXSystem::GetActiveNarrativeVFX() const
{
    return ActiveNarrativeVFX;
}

int32 UNarrativeVFXSystem::GetActiveVFXCount() const
{
    return ActiveNarrativeVFX.Num();
}

ENarrativeEmotionalState UNarrativeVFXSystem::GetCurrentEmotionalState() const
{
    return CurrentEmotionalState;
}

float UNarrativeVFXSystem::GetEmotionalIntensity() const
{
    return EmotionalIntensity;
}

ENarrativeStoryBeat UNarrativeVFXSystem::GetCurrentStoryBeat() const
{
    return CurrentStoryBeat;
}

float UNarrativeVFXSystem::GetStoryProgression() const
{
    return StoryProgressionValue;
}

void UNarrativeVFXSystem::DebugDrawNarrativeVFX(bool bDrawActive, bool bDrawEmotional, bool bDrawStory)
{
    if (!GetWorld()) return;
    
    if (bDrawActive)
    {
        for (const FNarrativeVFXInstance& Instance : ActiveNarrativeVFX)
        {
            DrawDebugSphere(GetWorld(), Instance.WorldLocation, 50.0f, 12, FColor::Green, false, 0.1f);
            DrawDebugString(GetWorld(), Instance.WorldLocation + FVector(0, 0, 60), 
                           Instance.VFXData.EffectName, nullptr, FColor::White, 0.1f);
        }
    }
    
    if (bDrawEmotional)
    {
        // Draw emotional state info near player
        if (PlayerController && PlayerController->GetPawn())
        {
            FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
            FString EmotionalInfo = FString::Printf(TEXT("Emotional State: %d\nIntensity: %.2f"), 
                                                   static_cast<int32>(CurrentEmotionalState), EmotionalIntensity);
            DrawDebugString(GetWorld(), PlayerLocation + FVector(0, 0, 200), 
                           EmotionalInfo, nullptr, FColor::Yellow, 0.1f);
        }
    }
    
    if (bDrawStory)
    {
        // Draw story progression info
        if (PlayerController && PlayerController->GetPawn())
        {
            FVector PlayerLocation = PlayerController->GetPawn()->GetActorLocation();
            FString StoryInfo = FString::Printf(TEXT("Story Beat: %d\nProgression: %.2f"), 
                                              static_cast<int32>(CurrentStoryBeat), StoryProgressionValue);
            DrawDebugString(GetWorld(), PlayerLocation + FVector(0, 0, 250), 
                           StoryInfo, nullptr, FColor::Cyan, 0.1f);
        }
    }
}

void UNarrativeVFXSystem::PrintNarrativeVFXStats()
{
    UE_LOG(LogNarrativeVFX, Warning, TEXT("=== Narrative VFX System Stats ==="));
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Active Narrative VFX: %d"), ActiveNarrativeVFX.Num());
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Active Character Emotions: %d"), ActiveCharacterEmotions.Num());
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Active Environmental Story VFX: %d"), ActiveEnvironmentalStoryVFX.Num());
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Current Emotional State: %d"), static_cast<int32>(CurrentEmotionalState));
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Emotional Intensity: %.2f"), EmotionalIntensity);
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Current Story Beat: %d"), static_cast<int32>(CurrentStoryBeat));
    UE_LOG(LogNarrativeVFX, Warning, TEXT("Story Progression: %.2f"), StoryProgressionValue);
    UE_LOG(LogNarrativeVFX, Warning, TEXT("System Active: %s"), bIsSystemActive ? TEXT("Yes") : TEXT("No"));
}

// Private helper methods implementation

void UNarrativeVFXSystem::InitializeEmotionalVFXPools()
{
    // Initialize VFX pools for different emotional states
    // This would be populated with actual Niagara systems in a real implementation
    UE_LOG(LogNarrativeVFX, Log, TEXT("Emotional VFX pools initialized"));
}

void UNarrativeVFXSystem::InitializeStoryBeatTracking()
{
    // Setup story beat tracking system
    UE_LOG(LogNarrativeVFX, Log, TEXT("Story beat tracking initialized"));
}

FNarrativeVFXData* UNarrativeVFXSystem::FindVFXForStoryBeat(ENarrativeStoryBeat StoryBeat)
{
    // In a real implementation, this would search through a data table or asset registry
    // For now, return a default VFX data structure
    static FNarrativeVFXData DefaultVFX;
    DefaultVFX.EffectName = FString::Printf(TEXT("StoryBeat_%d_VFX"), static_cast<int32>(StoryBeat));
    return &DefaultVFX;
}

FNarrativeVFXData* UNarrativeVFXSystem::FindVFXForCharacterEmotion(ENarrativeCharacterEmotion Emotion)
{
    // In a real implementation, this would search through a data table or asset registry
    static FNarrativeVFXData DefaultEmotionVFX;
    DefaultEmotionVFX.EffectName = FString::Printf(TEXT("CharacterEmotion_%d_VFX"), static_cast<int32>(Emotion));
    return &DefaultEmotionVFX;
}

int32 UNarrativeVFXSystem::GenerateVFXInstanceID()
{
    static int32 NextID = 1;
    return NextID++;
}

bool UNarrativeVFXSystem::SpawnNarrativeVFXInstance(FNarrativeVFXInstance& Instance)
{
    // In a real implementation, this would spawn actual Niagara components
    // For now, just mark as successfully spawned
    UE_LOG(LogNarrativeVFX, VeryVerbose, TEXT("Spawned narrative VFX instance: %s"), 
           *Instance.VFXData.EffectName);
    return true;
}

void UNarrativeVFXSystem::UpdateEmotionalTransitions(float DeltaTime)
{
    if (!bEmotionalTransitionInProgress) return;
    
    // Update emotional transition progress
    // Implementation would handle smooth transitions between emotional states
}

void UNarrativeVFXSystem::UpdateActiveNarrativeVFX(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update and cleanup expired VFX instances
    for (int32 i = ActiveNarrativeVFX.Num() - 1; i >= 0; i--)
    {
        FNarrativeVFXInstance& Instance = ActiveNarrativeVFX[i];
        
        // Check if VFX should be removed
        if (!Instance.bIsActive || (Instance.Duration > 0.0f && 
            CurrentTime - Instance.StartTime > Instance.Duration))
        {
            CleanupVFXInstance(Instance);
            ActiveNarrativeVFX.RemoveAt(i);
        }
    }
    
    // Update character emotion VFX
    UpdateCharacterEmotionVFX(DeltaTime);
    
    // Update environmental story VFX
    UpdateEnvironmentalStoryVFX();
}

void UNarrativeVFXSystem::UpdateStoryProgression(float DeltaTime)
{
    // Update story-related VFX based on progression
    // Implementation would handle dynamic story-driven effects
}

void UNarrativeVFXSystem::StartEmotionalTransition(ENarrativeEmotionalState FromState, ENarrativeEmotionalState ToState, float Duration)
{
    bEmotionalTransitionInProgress = true;
    // Implementation would handle smooth emotional state transitions
}

void UNarrativeVFXSystem::StartIntensityTransition(float TargetIntensity, float Duration)
{
    // Implementation would handle smooth intensity transitions
}

void UNarrativeVFXSystem::ApplyEmotionalStateVFX(ENarrativeEmotionalState State, float Intensity)
{
    // Implementation would apply VFX based on emotional state
}

void UNarrativeVFXSystem::UpdateEmotionalVFXIntensity()
{
    // Implementation would update intensity of active emotional VFX
}

void UNarrativeVFXSystem::UpdateCharacterEmotionVFX(float DeltaTime)
{
    float CurrentTime = GetWorld()->GetTimeSeconds();
    
    // Update and cleanup expired character emotion VFX
    for (int32 i = ActiveCharacterEmotions.Num() - 1; i >= 0; i--)
    {
        FCharacterEmotionVFX& EmotionVFX = ActiveCharacterEmotions[i];
        
        if (!EmotionVFX.bIsActive || 
            CurrentTime - EmotionVFX.StartTime > EmotionVFX.Duration)
        {
            // Remove associated VFX instance
            RemoveVFXInstance(EmotionVFX.VFXInstanceID);
            ActiveCharacterEmotions.RemoveAt(i);
        }
    }
}

void UNarrativeVFXSystem::UpdateEnvironmentalStoryVFX()
{
    // Implementation would update environmental story VFX
}

void UNarrativeVFXSystem::UpdateActiveNarrativeVFXSettings()
{
    // Apply new settings to active VFX instances
}

void UNarrativeVFXSystem::CullExcessiveNarrativeVFX()
{
    // Remove lowest priority VFX when over limit
    if (ActiveNarrativeVFX.Num() <= NarrativeSettings.MaxActiveNarrativeVFX) return;
    
    // Sort by priority and remove excess
    ActiveNarrativeVFX.Sort([](const FNarrativeVFXInstance& A, const FNarrativeVFXInstance& B) {
        return A.VFXData.Priority > B.VFXData.Priority; // Higher priority first
    });
    
    int32 ExcessCount = ActiveNarrativeVFX.Num() - NarrativeSettings.MaxActiveNarrativeVFX;
    for (int32 i = 0; i < ExcessCount; i++)
    {
        int32 RemoveIndex = ActiveNarrativeVFX.Num() - 1 - i;
        CleanupVFXInstance(ActiveNarrativeVFX[RemoveIndex]);
        ActiveNarrativeVFX.RemoveAt(RemoveIndex);
    }
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Culled %d excessive narrative VFX"), ExcessCount);
}

void UNarrativeVFXSystem::CleanupVFXInstance(FNarrativeVFXInstance& Instance)
{
    // Cleanup Niagara component and other resources
    Instance.bIsActive = false;
}

void UNarrativeVFXSystem::RemoveVFXInstance(int32 InstanceID)
{
    for (int32 i = 0; i < ActiveNarrativeVFX.Num(); i++)
    {
        if (ActiveNarrativeVFX[i].InstanceID == InstanceID)
        {
            CleanupVFXInstance(ActiveNarrativeVFX[i]);
            ActiveNarrativeVFX.RemoveAt(i);
            break;
        }
    }
}

void UNarrativeVFXSystem::CleanupNarrativeVFXSystem()
{
    // Cleanup all active VFX
    for (FNarrativeVFXInstance& Instance : ActiveNarrativeVFX)
    {
        CleanupVFXInstance(Instance);
    }
    
    ActiveNarrativeVFX.Empty();
    ActiveCharacterEmotions.Empty();
    ActiveEnvironmentalStoryVFX.Empty();
    
    bIsSystemActive = false;
    
    UE_LOG(LogNarrativeVFX, Log, TEXT("Narrative VFX System cleaned up"));
}

void UNarrativeVFXSystem::CleanupEmotionalVFX()
{
    // Remove emotional VFX instances
    for (int32 i = ActiveNarrativeVFX.Num() - 1; i >= 0; i--)
    {
        if (ActiveNarrativeVFX[i].EmotionalState != ENarrativeEmotionalState::Neutral)
        {
            CleanupVFXInstance(ActiveNarrativeVFX[i]);
            ActiveNarrativeVFX.RemoveAt(i);
        }
    }
}

void UNarrativeVFXSystem::CleanupStoryMomentVFX()
{
    // Remove story moment VFX instances
    for (int32 i = ActiveNarrativeVFX.Num() - 1; i >= 0; i--)
    {
        if (ActiveNarrativeVFX[i].StoryBeat != ENarrativeStoryBeat::None)
        {
            CleanupVFXInstance(ActiveNarrativeVFX[i]);
            ActiveNarrativeVFX.RemoveAt(i);
        }
    }
    
    ActiveEnvironmentalStoryVFX.Empty();
}

void UNarrativeVFXSystem::CleanupCharacterEmotionVFX()
{
    // Remove character emotion VFX
    for (FCharacterEmotionVFX& EmotionVFX : ActiveCharacterEmotions)
    {
        RemoveVFXInstance(EmotionVFX.VFXInstanceID);
    }
    
    ActiveCharacterEmotions.Empty();
}