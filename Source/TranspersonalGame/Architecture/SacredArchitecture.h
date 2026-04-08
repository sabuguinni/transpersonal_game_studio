#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SceneComponent.h"
#include "Engine/Engine.h"
#include "SacredArchitecture.generated.h"

UENUM(BlueprintType)
enum class EArchitecturalStyle : uint8
{
    Temple         UMETA(DisplayName = "Sacred Temple"),
    Mandala        UMETA(DisplayName = "Mandala Structure"),
    Spiral         UMETA(DisplayName = "Spiral Tower"),
    Crystal        UMETA(DisplayName = "Crystal Formation"),
    Organic        UMETA(DisplayName = "Organic Flow"),
    Geometric      UMETA(DisplayName = "Sacred Geometry")
};

UENUM(BlueprintType)
enum class EConsciousnessLevel : uint8
{
    Ego            UMETA(DisplayName = "Ego Level"),
    Personal       UMETA(DisplayName = "Personal Unconscious"),
    Collective     UMETA(DisplayName = "Collective Unconscious"),
    Archetypal     UMETA(DisplayName = "Archetypal"),
    Cosmic         UMETA(DisplayName = "Cosmic Consciousness"),
    Unity          UMETA(DisplayName = "Unity Consciousness")
};

USTRUCT(BlueprintType)
struct FArchitecturalTransformation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    EConsciousnessLevel TriggerLevel;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ScaleMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FRotator RotationOffset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EmissiveColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MaterialOpacity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float TransformationDuration;

    FArchitecturalTransformation()
    {
        TriggerLevel = EConsciousnessLevel::Ego;
        ScaleMultiplier = FVector(1.0f);
        RotationOffset = FRotator::ZeroRotator;
        EmissiveColor = FLinearColor::White;
        MaterialOpacity = 1.0f;
        TransformationDuration = 3.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ASacredArchitecture : public AActor
{
    GENERATED_BODY()

public:
    ASacredArchitecture();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* MainStructure;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* DetailElements;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    EArchitecturalStyle ArchitecturalStyle;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    TArray<FArchitecturalTransformation> ConsciousnessTransformations;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    bool bRespondsToConsciousness;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    float BaseResonanceFrequency;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Architecture")
    FVector SacredProportions;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EConsciousnessLevel CurrentConsciousnessLevel;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransforming;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TransformationProgress;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void SetConsciousnessLevel(EConsciousnessLevel NewLevel);

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void TriggerSacredResonance();

    UFUNCTION(BlueprintCallable, Category = "Architecture")
    void ApplyGoldenRatioProportions();

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnConsciousnessLevelChanged(EConsciousnessLevel OldLevel, EConsciousnessLevel NewLevel);

    UFUNCTION(BlueprintImplementableEvent, Category = "Architecture")
    void OnTransformationComplete();

private:
    void UpdateArchitecturalTransformation(float DeltaTime);
    void CalculateSacredGeometry();
    
    FArchitecturalTransformation* GetTransformationForLevel(EConsciousnessLevel Level);
    
    // Transformation interpolation
    FVector InitialScale;
    FRotator InitialRotation;
    FLinearColor InitialEmissiveColor;
    float InitialOpacity;
    
    FVector TargetScale;
    FRotator TargetRotation;
    FLinearColor TargetEmissiveColor;
    float TargetOpacity;
    
    float CurrentTransformationTime;
    float TotalTransformationTime;
};