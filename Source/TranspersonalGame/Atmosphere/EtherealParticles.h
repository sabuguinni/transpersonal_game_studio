#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "NiagaraComponent.h"
#include "NiagaraSystem.h"
#include "Engine/World.h"
#include "EtherealParticles.generated.h"

UENUM(BlueprintType)
enum class EEtherealParticleType : uint8
{
    Dust            UMETA(DisplayName = "Cosmic Dust"),
    Orbs            UMETA(DisplayName = "Light Orbs"),
    Energy          UMETA(DisplayName = "Energy Streams"),
    Chakra          UMETA(DisplayName = "Chakra Particles"),
    Aura            UMETA(DisplayName = "Aura Field"),
    Sacred          UMETA(DisplayName = "Sacred Geometry"),
    Meditation      UMETA(DisplayName = "Meditation Wisps"),
    Transcendence   UMETA(DisplayName = "Transcendence Rays")
};

USTRUCT(BlueprintType)
struct FEtherealParticleData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    UNiagaraSystem* ParticleSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnLocation = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector SpawnScale = FVector::OneVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float BaseIntensity = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SpawnRate = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor BaseColor = FLinearColor::White;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float LifetimeMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float VelocityMultiplier = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRespondsToConsciousness = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRespondsToMeditation = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bRespondsToEmotions = false;
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEtherealParticles : public UActorComponent
{
    GENERATED_BODY()

public:
    UEtherealParticles();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void SpawnParticleSystem(EEtherealParticleType ParticleType, FVector Location, float Intensity = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void UpdateConsciousnessLevel(float ConsciousnessLevel);

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void SetMeditationState(bool bIsMeditating, float MeditationDepth = 1.0f);

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void TriggerEnlightenmentBurst(FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void SetEmotionalState(float Joy, float Peace, float Love, float Compassion);

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void CreateAuraField(FVector CenterLocation, float Radius, FLinearColor AuraColor);

    UFUNCTION(BlueprintCallable, Category = "Ethereal Particles")
    void UpdateSacredGeometry(bool bVisible, float ComplexityLevel = 1.0f);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Particle Systems")
    TMap<EEtherealParticleType, FEtherealParticleData> ParticleDefinitions;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Active Systems")
    TArray<UNiagaraComponent*> ActiveParticleSystems;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Consciousness")
    float CurrentConsciousnessLevel = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    bool bIsMeditating = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float MeditationDepth = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalJoy = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalPeace = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalLove = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Emotions")
    float EmotionalCompassion = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UNiagaraComponent* AuraFieldSystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects")
    UNiagaraComponent* SacredGeometrySystem = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float UpdateFrequency = 0.1f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Settings")
    float MaxParticleSystems = 20;

private:
    float LastUpdateTime = 0.0f;
    TMap<EEtherealParticleType, UNiagaraComponent*> PersistentSystems;

    void InitializeParticleDefinitions();
    void UpdateAllParticleSystems();
    void UpdateParticleSystem(UNiagaraComponent* System, const FEtherealParticleData& Data);
    UNiagaraComponent* CreateParticleSystem(const FEtherealParticleData& Data, FVector Location);
    void CleanupInactiveSystems();
    float CalculateIntensityMultiplier(const FEtherealParticleData& Data);
    FLinearColor CalculateParticleColor(const FEtherealParticleData& Data);
};