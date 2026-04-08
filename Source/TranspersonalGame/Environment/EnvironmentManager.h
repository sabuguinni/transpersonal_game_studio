// Copyright Transpersonal Game Studio. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/MaterialParameterCollection.h"
#include "../Consciousness/ConsciousnessComponent.h"
#include "EnvironmentManager.generated.h"

UENUM(BlueprintType)
enum class EEnvironmentState : uint8
{
    Neutral     UMETA(DisplayName = "Neutral"),
    Meditative  UMETA(DisplayName = "Meditative"), 
    Emotional   UMETA(DisplayName = "Emotional"),
    Spiritual   UMETA(DisplayName = "Spiritual"),
    Transcendent UMETA(DisplayName = "Transcendent")
};

USTRUCT(BlueprintType)
struct FEnvironmentResponse
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientColor = FLinearColor::White;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AuraColor = FLinearColor(0.5f, 0.8f, 1.0f, 0.3f);
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EmissionIntensity = 0.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float FogDensity = 0.02f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float ParticleIntensity = 1.0f;
};

UCLASS()
class TRANSPERSONALGAME_API AEnvironmentManager : public AActor
{
    GENERATED_BODY()
    
public:    
    AEnvironmentManager();

protected:
    virtual void BeginPlay() override;

public:    
    virtual void Tick(float DeltaTime) override;

    // Environment Response System
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void UpdateEnvironmentState(float ConsciousnessLevel, float EmotionalResonance, float SpiritualAlignment);
    
    UFUNCTION(BlueprintCallable, Category = "Environment")
    void SetEnvironmentState(EEnvironmentState NewState);
    
    UFUNCTION(BlueprintImplementableEvent, Category = "Environment")
    void OnEnvironmentStateChanged(EEnvironmentState NewState);

protected:
    // Core Components
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    class USceneComponent* RootSceneComponent;
    
    // Material Parameter Collection for global environment control
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment")
    class UMaterialParameterCollection* EnvironmentMPC;
    
    // Environment States Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Environment States")
    TMap<EEnvironmentState, FEnvironmentResponse> EnvironmentResponses;
    
    // Current State
    UPROPERTY(BlueprintReadOnly, Category = "Environment")
    EEnvironmentState CurrentEnvironmentState = EEnvironmentState::Neutral;
    
    // Transition Settings
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionSpeed = 2.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float ConsciousnessThreshold = 0.3f;
    
    // Current Values for Interpolation
    FEnvironmentResponse CurrentResponse;
    FEnvironmentResponse TargetResponse;
    
    // Internal Functions
    void InitializeEnvironmentResponses();
    void UpdateMaterialParameters();
    EEnvironmentState DetermineEnvironmentState(float ConsciousnessLevel, float EmotionalResonance, float SpiritualAlignment);
    void InterpolateToTarget(float DeltaTime);
    
    // Player Reference
    UPROPERTY()
    class ATranspersonalGameCharacter* PlayerCharacter;
    
    UPROPERTY()
    class UConsciousnessComponent* PlayerConsciousness;
};