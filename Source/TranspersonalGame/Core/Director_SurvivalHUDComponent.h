// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/Engine.h"
#include "Director_SurvivalHUDComponent.generated.h"

DECLARE_LOG_CATEGORY_EXTERN(LogDirectorSurvivalHUD, Log, All);

/**
 * Survival HUD Component for displaying health, hunger, thirst, stamina bars
 * Created by Studio Director Agent #1 for playable prototype
 */
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UDirector_SurvivalHUDComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UDirector_SurvivalHUDComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // HUD Display Properties
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival HUD")
    bool bShowHUD = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival HUD")
    FVector2D HUDPosition = FVector2D(50.0f, 50.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival HUD")
    FVector2D BarSize = FVector2D(200.0f, 20.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival HUD")
    float BarSpacing = 30.0f;

    // Survival Stats to Display
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Health = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxHealth = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Hunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxHunger = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Thirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxThirst = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float Stamina = 100.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Stats")
    float MaxStamina = 100.0f;

    // HUD Colors
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Colors")
    FLinearColor HealthColor = FLinearColor::Red;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Colors")
    FLinearColor HungerColor = FLinearColor(1.0f, 0.5f, 0.0f); // Orange

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Colors")
    FLinearColor ThirstColor = FLinearColor::Blue;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Colors")
    FLinearColor StaminaColor = FLinearColor::Green;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HUD Colors")
    FLinearColor BackgroundColor = FLinearColor(0.0f, 0.0f, 0.0f, 0.5f);

    // Functions
    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void UpdateHealthBar(float NewHealth, float NewMaxHealth);

    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void UpdateHungerBar(float NewHunger, float NewMaxHunger);

    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void UpdateThirstBar(float NewThirst, float NewMaxThirst);

    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void UpdateStaminaBar(float NewStamina, float NewMaxStamina);

    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void UpdateAllStats(float NewHealth, float NewHunger, float NewThirst, float NewStamina);

    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void SetHUDVisibility(bool bVisible);

    UFUNCTION(BlueprintCallable, Category = "Survival HUD")
    void DrawSurvivalBars();

private:
    // Helper functions
    void DrawStatBar(const FString& StatName, float CurrentValue, float MaxValue, 
                     const FLinearColor& BarColor, const FVector2D& Position);

    float GetStatPercentage(float Current, float Max) const;

    // Internal state
    bool bIsInitialized = false;
    float UpdateInterval = 0.1f; // Update HUD every 0.1 seconds
    float TimeSinceLastUpdate = 0.0f;
};