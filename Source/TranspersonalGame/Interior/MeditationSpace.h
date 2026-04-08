#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/PointLightComponent.h"
#include "Components/AudioComponent.h"
#include "Components/SphereComponent.h"
#include "Sound/SoundCue.h"
#include "MeditationSpace.generated.h"

UENUM(BlueprintType)
enum class EMeditationSpaceType : uint8
{
    Zen            UMETA(DisplayName = "Zen Garden"),
    Crystal        UMETA(DisplayName = "Crystal Cave"),
    Forest         UMETA(DisplayName = "Sacred Grove"),
    Temple         UMETA(DisplayName = "Inner Temple"),
    Void           UMETA(DisplayName = "Void Space"),
    Mandala        UMETA(DisplayName = "Mandala Chamber")
};

UENUM(BlueprintType)
enum class EMeditationState : uint8
{
    Entering       UMETA(DisplayName = "Entering"),
    Settling       UMETA(DisplayName = "Settling"),
    Focused        UMETA(DisplayName = "Focused"),
    Deep           UMETA(DisplayName = "Deep Meditation"),
    Transcendent   UMETA(DisplayName = "Transcendent"),
    Returning      UMETA(DisplayName = "Returning")
};

USTRUCT(BlueprintType)
struct FMeditationAmbience
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor AmbientLightColor;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float AmbientLightIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    USoundCue* AmbientSound;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float SoundVolume;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FVector ParticleEmissionRate;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float EnergyFieldIntensity;

    FMeditationAmbience()
    {
        AmbientLightColor = FLinearColor(0.8f, 0.9f, 1.0f, 1.0f);
        AmbientLightIntensity = 1.0f;
        AmbientSound = nullptr;
        SoundVolume = 0.5f;
        ParticleEmissionRate = FVector(10.0f, 10.0f, 10.0f);
        EnergyFieldIntensity = 1.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AMeditationSpace : public AActor
{
    GENERATED_BODY()

public:
    AMeditationSpace();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USphereComponent* MeditationZone;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* FloorMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CeilingMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* WallMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CenterPiece;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UPointLightComponent* CentralLight;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UAudioComponent* AmbientAudio;

    // Configuration
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation Space")
    EMeditationSpaceType SpaceType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation Space")
    TMap<EMeditationState, FMeditationAmbience> StateAmbiences;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation Space")
    float MeditationZoneRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation Space")
    bool bAutoDetectMeditation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation Space")
    float StateTransitionDuration;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Meditation Space")
    TArray<FVector> SacredSymbolPositions;

    // Current state
    UPROPERTY(BlueprintReadOnly, Category = "State")
    EMeditationState CurrentMeditationState;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bPlayerInZone;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float MeditationDepth;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    float TimeInMeditation;

    UPROPERTY(BlueprintReadOnly, Category = "State")
    bool bIsTransitioning;

public:
    virtual void Tick(float DeltaTime) override;

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void EnterMeditationSpace(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void ExitMeditationSpace(AActor* Player);

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void SetMeditationState(EMeditationState NewState);

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void UpdateMeditationDepth(float NewDepth);

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void ActivateSacredGeometry();

    UFUNCTION(BlueprintCallable, Category = "Meditation")
    void CreateEnergyField();

    UFUNCTION(BlueprintImplementableEvent, Category = "Meditation")
    void OnPlayerEnteredMeditation(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Meditation")
    void OnPlayerExitedMeditation(AActor* Player);

    UFUNCTION(BlueprintImplementableEvent, Category = "Meditation")
    void OnMeditationStateChanged(EMeditationState OldState, EMeditationState NewState);

    UFUNCTION(BlueprintImplementableEvent, Category = "Meditation")
    void OnMeditationDepthChanged(float OldDepth, float NewDepth);

protected:
    UFUNCTION()
    void OnMeditationZoneBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                     UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex, 
                                     bool bFromSweep, const FHitResult& SweepResult);

    UFUNCTION()
    void OnMeditationZoneEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, 
                                   UPrimitiveComponent* OtherComponent, int32 OtherBodyIndex);

private:
    void InitializeStateAmbiences();
    void TransitionToState(EMeditationState NewState);
    void UpdateAmbience(float DeltaTime);
    void UpdateMeditationProgression(float DeltaTime);
    void CreateSacredSymbols();
    
    // Transition interpolation
    FMeditationAmbience CurrentAmbience;
    FMeditationAmbience TargetAmbience;
    float TransitionProgress;
    float TransitionTimer;
    
    // Meditation progression
    float MeditationTimer;
    float LastDepthUpdate;
    
    // Energy field
    bool bEnergyFieldActive;
    float EnergyFieldPhase;
};