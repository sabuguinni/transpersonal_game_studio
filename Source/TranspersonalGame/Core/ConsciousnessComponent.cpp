#include "ConsciousnessComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"

UConsciousnessComponent::UConsciousnessComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.bStartWithTickEnabled = true;
    
    // Initialize default consciousness state
    CurrentLevel.State = EConsciousnessState::Ordinary;
    CurrentLevel.Intensity = 10.0f;
    CurrentLevel.TransitionSpeed = BaseTransitionSpeed;
    
    TargetLevel = CurrentLevel;
}

void UConsciousnessComponent::BeginPlay()
{
    Super::BeginPlay();
    
    // Broadcast initial state
    BroadcastConsciousnessChange();
}

void UConsciousnessComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (bAutoTransition)
    {
        UpdateConsciousnessTransition(DeltaTime);
    }
}

void UConsciousnessComponent::SetConsciousnessState(EConsciousnessState NewState, float TargetIntensity)
{
    TargetLevel.State = NewState;
    TargetLevel.Intensity = FMath::Clamp(TargetIntensity, 0.0f, 100.0f);
    
    // Immediate state change for dramatic shifts
    if (FMath::Abs(static_cast<int32>(NewState) - static_cast<int32>(CurrentLevel.State)) > 1)
    {
        CurrentLevel.State = NewState;
        BroadcastConsciousnessChange();
    }
}

void UConsciousnessComponent::ModifyConsciousnessIntensity(float Delta)
{
    float NewIntensity = FMath::Clamp(CurrentLevel.Intensity + Delta, 0.0f, 100.0f);
    
    // Check for state transitions based on intensity thresholds
    EConsciousnessState NewState = CurrentLevel.State;
    
    if (NewIntensity >= 80.0f && CurrentLevel.State != EConsciousnessState::Unity)
    {
        NewState = EConsciousnessState::Unity;
    }
    else if (NewIntensity >= 60.0f && NewIntensity < 80.0f && CurrentLevel.State != EConsciousnessState::Transcendent)
    {
        NewState = EConsciousnessState::Transcendent;
    }
    else if (NewIntensity >= 40.0f && NewIntensity < 60.0f && CurrentLevel.State != EConsciousnessState::Expanded)
    {
        NewState = EConsciousnessState::Expanded;
    }
    else if (NewIntensity < 40.0f && CurrentLevel.State != EConsciousnessState::Ordinary)
    {
        NewState = EConsciousnessState::Ordinary;
    }
    
    SetConsciousnessState(NewState, NewIntensity);
}

void UConsciousnessComponent::TriggerConsciousnessShift(const FGameplayTag& TriggerTag)
{
    // Handle specific consciousness triggers
    if (TriggerTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Consciousness.Trigger.Meditation"))))
    {
        ModifyConsciousnessIntensity(15.0f);
    }
    else if (TriggerTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Consciousness.Trigger.Trauma"))))
    {
        SetConsciousnessState(EConsciousnessState::Void, 90.0f);
    }
    else if (TriggerTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Consciousness.Trigger.Insight"))))
    {
        ModifyConsciousnessIntensity(25.0f);
    }
    else if (TriggerTag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Consciousness.Trigger.Integration"))))
    {
        SetConsciousnessState(EConsciousnessState::Unity, 85.0f);
    }
}

void UConsciousnessComponent::UpdateConsciousnessTransition(float DeltaTime)
{
    bool bStateChanged = false;
    
    // Smooth intensity transition
    if (!FMath::IsNearlyEqual(CurrentLevel.Intensity, TargetLevel.Intensity, 0.1f))
    {
        float IntensityDelta = (TargetLevel.Intensity - CurrentLevel.Intensity) * CurrentLevel.TransitionSpeed * DeltaTime;
        CurrentLevel.Intensity = FMath::FInterpTo(CurrentLevel.Intensity, TargetLevel.Intensity, DeltaTime, CurrentLevel.TransitionSpeed);
        bStateChanged = true;
    }
    
    // State transition
    if (CurrentLevel.State != TargetLevel.State)
    {
        CurrentLevel.State = TargetLevel.State;
        bStateChanged = true;
    }
    
    if (bStateChanged)
    {
        BroadcastConsciousnessChange();
    }
}

void UConsciousnessComponent::BroadcastConsciousnessChange()
{
    OnConsciousnessChanged.Broadcast(CurrentLevel.State, CurrentLevel.Intensity);
    
    // Debug output
    if (GEngine)
    {
        FString StateString = UEnum::GetValueAsString(CurrentLevel.State);
        FString DebugMessage = FString::Printf(TEXT("Consciousness: %s (%.1f%%)"), *StateString, CurrentLevel.Intensity);
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, DebugMessage);
    }
}