#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "Particles/ParticleSystemComponent.h"
#include "../Core/ConsciousnessSystem.h"
#include "MeditationMechanics.generated.h"

UENUM(BlueprintType)
enum class EMeditationType : uint8
{
    Mindfulness     UMETA(DisplayName = "Mindfulness Meditation"),
    Transcendental  UMETA(DisplayName = "Transcendental Meditation"),
    Loving_Kindness UMETA(DisplayName = "Loving-Kindness Meditation"),
    Zen             UMETA(DisplayName = "Zen Meditation"),
    Chakra          UMETA(DisplayName = "Chakra Meditation"),
    Breathwork      UMETA(DisplayName = "Breathwork"),
    Walking         UMETA(DisplayName = "Walking Meditation")
};

USTRUCT(BlueprintType)
struct FMeditationSession
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    EMeditationType Type;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float Duration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float Depth;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float Focus;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    bool bGroupMeditation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    int32 ParticipantCount;

    FMeditationSession()
    {
        Type = EMeditationType::Mindfulness;
        Duration = 0.0f;
        Depth = 0.0f;
        Focus = 50.0f;
        bGroupMeditation = false;
        ParticipantCount = 1;
    }
};

UCLASS()
class TRANSPERSONALGAME_API AMeditationZone : public AActor
{
    GENERATED_BODY()

public:
    AMeditationZone();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* MeditationArea;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* ZoneMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UParticleSystemComponent* AuraEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    EMeditationType ZoneType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float ConsciousnessAmplification;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float ZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
    USoundCue* AmbientMeditationSound;

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void ActivateZone();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void DeactivateZone();

    UFUNCTION()
    void OnPlayerEnterZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnPlayerExitZone(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
    TArray<AActor*> MeditatingPlayers;
    bool bZoneActive;
    float ZoneIntensity;

    void UpdateZoneEffects(float DeltaTime);
    void ApplyMeditationBonus(AActor* Player, float DeltaTime);
};

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UMeditationComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UMeditationComponent();

protected:
    virtual void BeginPlay() override;

public:
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    FMeditationSession CurrentSession;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float BreathingRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    float HeartRateVariability;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation")
    bool bInMeditation;

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void StartMeditation(EMeditationType Type);

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void EndMeditation();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void UpdateBreathing(float InhaleTime, float ExhaleTime);

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void ProcessMindfulness(float AttentionLevel);

    UFUNCTION(BlueprintPure, Category = "Meditation")
    float GetMeditationQuality() const;

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void JoinGroupMeditation(const TArray<AActor*>& Participants);

private:
    float SessionTimer;
    float FocusAccumulator;
    float DistractionPenalty;
    UConsciousnessComponent* ConsciousnessComp;

    void ProcessMeditationType(float DeltaTime);
    void CalculateMeditationDepth(float DeltaTime);
    void ApplyConsciousnessEffects();
};

UCLASS()
class TRANSPERSONALGAME_API UBreathingController : public UObject
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float InhaleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float ExhaleTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    float HoldTime;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Breathing")
    bool bCoherentBreathing;

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void StartBreathingPattern();

    UFUNCTION(BlueprintCallable, Category = "Breathing")
    void UpdateBreathingCycle(float DeltaTime);

    UFUNCTION(BlueprintPure, Category = "Breathing")
    float GetBreathingCoherence() const;

private:
    float BreathCycleTimer;
    float CoherenceScore;
    bool bInhaling;
    bool bExhaling;
    bool bHolding;

    void CalculateCoherence();
};