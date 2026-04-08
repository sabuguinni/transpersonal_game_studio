// TranscendentalLighting.h
// Sistema de iluminação transcendental para experiências espirituais
#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Components/LightComponent.h"
#include "TranscendentalLighting.generated.h"

UENUM(BlueprintType)
enum class ELightingState : uint8
{
    Dormant         UMETA(DisplayName = "Dormant"),
    Awakening       UMETA(DisplayName = "Awakening"),
    Transcendent    UMETA(DisplayName = "Transcendent"),
    Unity           UMETA(DisplayName = "Unity"),
    Void            UMETA(DisplayName = "Void")
};

UENUM(BlueprintType)
enum class ELightPattern : uint8
{
    Static          UMETA(DisplayName = "Static"),
    Breathing       UMETA(DisplayName = "Breathing"),
    Pulsing         UMETA(DisplayName = "Pulsing"),
    Flowing         UMETA(DisplayName = "Flowing"),
    Spiraling       UMETA(DisplayName = "Spiraling"),
    Cascading       UMETA(DisplayName = "Cascading")
};

USTRUCT(BlueprintType)
struct FLightingConfiguration
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BaseColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Intensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Temperature = 6500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ELightPattern Pattern = ELightPattern::Static;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AnimationSpeed = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VolumetricScattering = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bCastVolumetricShadow = true;
};

UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UTranscendentalLighting : public UActorComponent
{
    GENERATED_BODY()

public:
    UTranscendentalLighting();

protected:
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    // Configurações de iluminação por estado de consciência
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Lighting States")
    TMap<ELightingState, FLightingConfiguration> LightingStates;

    // Estado atual
    UPROPERTY(BlueprintReadOnly, Category = "Current State")
    ELightingState CurrentState = ELightingState::Dormant;

    // Componentes de luz
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    TArray<ULightComponent*> ManagedLights;

    // Configurações de transição
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    float TransitionDuration = 2.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Transitions")
    UCurveFloat* TransitionCurve;

    // Variáveis de animação
    float AnimationTime = 0.0f;
    bool bIsTransitioning = false;
    float TransitionProgress = 0.0f;
    ELightingState PreviousState;
    ELightingState TargetState;

public:
    // Funções principais
    UFUNCTION(BlueprintCallable, Category = "Transcendental Lighting")
    void SetLightingState(ELightingState NewState);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Lighting")
    void RegisterLight(ULightComponent* LightComponent);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Lighting")
    void UnregisterLight(ULightComponent* LightComponent);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Lighting")
    void SetGlobalIntensityMultiplier(float Multiplier);

    UFUNCTION(BlueprintCallable, Category = "Transcendental Lighting")
    void TriggerLightingEvent(const FString& EventName);

    // Efeitos especiais
    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateDivineRay(FVector StartLocation, FVector EndLocation, float Duration = 5.0f);

    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateAuraEffect(AActor* TargetActor, FLinearColor AuraColor, float Radius = 200.0f);

    UFUNCTION(BlueprintCallable, Category = "Special Effects")
    void CreateEnergyVortex(FVector Location, float Radius = 300.0f, float Duration = 10.0f);

private:
    // Funções internas
    void UpdateLightingAnimation(float DeltaTime);
    void ApplyLightingConfiguration(const FLightingConfiguration& Config, float Alpha = 1.0f);
    void UpdateTransition(float DeltaTime);
    FLinearColor InterpolateLightColor(const FLinearColor& ColorA, const FLinearColor& ColorB, float Alpha);
    void ApplyLightPattern(ULightComponent* Light, ELightPattern Pattern, float Time);

    // Multiplicador global
    float GlobalIntensityMultiplier = 1.0f;
};