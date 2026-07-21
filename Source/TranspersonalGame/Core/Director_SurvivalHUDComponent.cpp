// Copyright Transpersonal Game Studio. All Rights Reserved.

#include "Director_SurvivalHUDComponent.h"
#include "Engine/Canvas.h"
#include "Engine/Engine.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/HUD.h"

DEFINE_LOG_CATEGORY(LogDirectorSurvivalHUD);

UDirector_SurvivalHUDComponent::UDirector_SurvivalHUDComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
    PrimaryComponentTick.TickInterval = UpdateInterval;
    
    // Initialize default values
    Health = 100.0f;
    MaxHealth = 100.0f;
    Hunger = 100.0f;
    MaxHunger = 100.0f;
    Thirst = 100.0f;
    MaxThirst = 100.0f;
    Stamina = 100.0f;
    MaxStamina = 100.0f;
    
    bShowHUD = true;
    bIsInitialized = false;
    TimeSinceLastUpdate = 0.0f;
    
    UE_LOG(LogDirectorSurvivalHUD, Log, TEXT("Director_SurvivalHUDComponent created"));
}

void UDirector_SurvivalHUDComponent::BeginPlay()
{
    Super::BeginPlay();
    
    bIsInitialized = true;
    
    UE_LOG(LogDirectorSurvivalHUD, Warning, TEXT("Survival HUD Component initialized for: %s"), 
           *GetOwner()->GetName());
}

void UDirector_SurvivalHUDComponent::TickComponent(float DeltaTime, ELevelTick TickType, 
                                                   FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    
    if (!bIsInitialized || !bShowHUD)
    {
        return;
    }
    
    TimeSinceLastUpdate += DeltaTime;
    
    if (TimeSinceLastUpdate >= UpdateInterval)
    {
        DrawSurvivalBars();
        TimeSinceLastUpdate = 0.0f;
    }
}

void UDirector_SurvivalHUDComponent::UpdateHealthBar(float NewHealth, float NewMaxHealth)
{
    Health = FMath::Clamp(NewHealth, 0.0f, NewMaxHealth);
    MaxHealth = FMath::Max(NewMaxHealth, 1.0f);
    
    UE_LOG(LogDirectorSurvivalHUD, VeryVerbose, TEXT("Health updated: %f/%f"), Health, MaxHealth);
}

void UDirector_SurvivalHUDComponent::UpdateHungerBar(float NewHunger, float NewMaxHunger)
{
    Hunger = FMath::Clamp(NewHunger, 0.0f, NewMaxHunger);
    MaxHunger = FMath::Max(NewMaxHunger, 1.0f);
    
    UE_LOG(LogDirectorSurvivalHUD, VeryVerbose, TEXT("Hunger updated: %f/%f"), Hunger, MaxHunger);
}

void UDirector_SurvivalHUDComponent::UpdateThirstBar(float NewThirst, float NewMaxThirst)
{
    Thirst = FMath::Clamp(NewThirst, 0.0f, NewMaxThirst);
    MaxThirst = FMath::Max(NewMaxThirst, 1.0f);
    
    UE_LOG(LogDirectorSurvivalHUD, VeryVerbose, TEXT("Thirst updated: %f/%f"), Thirst, MaxThirst);
}

void UDirector_SurvivalHUDComponent::UpdateStaminaBar(float NewStamina, float NewMaxStamina)
{
    Stamina = FMath::Clamp(NewStamina, 0.0f, NewMaxStamina);
    MaxStamina = FMath::Max(NewMaxStamina, 1.0f);
    
    UE_LOG(LogDirectorSurvivalHUD, VeryVerbose, TEXT("Stamina updated: %f/%f"), Stamina, MaxStamina);
}

void UDirector_SurvivalHUDComponent::UpdateAllStats(float NewHealth, float NewHunger, 
                                                     float NewThirst, float NewStamina)
{
    UpdateHealthBar(NewHealth, MaxHealth);
    UpdateHungerBar(NewHunger, MaxHunger);
    UpdateThirstBar(NewThirst, MaxThirst);
    UpdateStaminaBar(NewStamina, MaxStamina);
}

void UDirector_SurvivalHUDComponent::SetHUDVisibility(bool bVisible)
{
    bShowHUD = bVisible;
    UE_LOG(LogDirectorSurvivalHUD, Log, TEXT("HUD visibility set to: %s"), 
           bVisible ? TEXT("Visible") : TEXT("Hidden"));
}

void UDirector_SurvivalHUDComponent::DrawSurvivalBars()
{
    if (!bShowHUD)
    {
        return;
    }
    
    // Get the player controller and HUD
    if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
    {
        if (APlayerController* PC = Cast<APlayerController>(OwnerPawn->GetController()))
        {
            if (AHUD* HUD = PC->GetHUD())
            {
                // Draw each survival bar
                FVector2D CurrentPosition = HUDPosition;
                
                // Health Bar
                DrawStatBar(TEXT("Health"), Health, MaxHealth, HealthColor, CurrentPosition);
                CurrentPosition.Y += BarSpacing;
                
                // Hunger Bar
                DrawStatBar(TEXT("Hunger"), Hunger, MaxHunger, HungerColor, CurrentPosition);
                CurrentPosition.Y += BarSpacing;
                
                // Thirst Bar
                DrawStatBar(TEXT("Thirst"), Thirst, MaxThirst, ThirstColor, CurrentPosition);
                CurrentPosition.Y += BarSpacing;
                
                // Stamina Bar
                DrawStatBar(TEXT("Stamina"), Stamina, MaxStamina, StaminaColor, CurrentPosition);
            }
        }
    }
}

void UDirector_SurvivalHUDComponent::DrawStatBar(const FString& StatName, float CurrentValue, 
                                                  float MaxValue, const FLinearColor& BarColor, 
                                                  const FVector2D& Position)
{
    // This is a simplified version - in a real implementation, 
    // you would use Canvas or UMG widgets for proper rendering
    float Percentage = GetStatPercentage(CurrentValue, MaxValue);
    
    // Log the stat for debugging (in a real game, this would render to screen)
    UE_LOG(LogDirectorSurvivalHUD, VeryVerbose, TEXT("%s: %.1f/%.1f (%.1f%%)"), 
           *StatName, CurrentValue, MaxValue, Percentage * 100.0f);
    
    // In a complete implementation, you would:
    // 1. Get the Canvas from the HUD
    // 2. Draw background rectangle
    // 3. Draw filled rectangle based on percentage
    // 4. Draw text label
    
    // For now, we'll use debug strings to show the values
    if (GEngine)
    {
        FString DebugString = FString::Printf(TEXT("%s: %.0f/%.0f"), 
                                              *StatName, CurrentValue, MaxValue);
        
        // Display on screen for 0.1 seconds (one frame)
        GEngine->AddOnScreenDebugMessage(
            FName(*StatName).GetComparisonIndex(), 
            0.1f, 
            BarColor.ToFColor(true), 
            DebugString
        );
    }
}

float UDirector_SurvivalHUDComponent::GetStatPercentage(float Current, float Max) const
{
    if (Max <= 0.0f)
    {
        return 0.0f;
    }
    
    return FMath::Clamp(Current / Max, 0.0f, 1.0f);
}