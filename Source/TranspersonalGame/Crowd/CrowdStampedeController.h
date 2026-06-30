#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CrowdSimulationTypes.h"
#include "CrowdStampedeController.generated.h"

// ── Stampede trigger source ──────────────────────────────────────────────────
UENUM(BlueprintType)
enum class ECrowd_StampedeTrigger : uint8
{
    PlayerProximity     UMETA(DisplayName = "Player Proximity"),
    PredatorDetected    UMETA(DisplayName = "Predator Detected"),
    LoudNoise           UMETA(DisplayName = "Loud Noise"),
    Explosion           UMETA(DisplayName = "Explosion"),
    LightningStrike     UMETA(DisplayName = "Lightning Strike"),
    AlphaFlee           UMETA(DisplayName = "Alpha Flee Signal")
};

// ── Per-wave propagation data ────────────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCrowd_StampedeWave
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    FVector Origin = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float PropagationRadius = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float PropagationSpeed = 1200.0f;   // cm/s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float PanicIntensity = 1.0f;        // 0..1 — decays with distance

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float ElapsedTime = 0.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    bool bActive = false;
};

// ── Danger zone for player avoidance ────────────────────────────────────────
USTRUCT(BlueprintType)
struct FCrowd_DangerZone
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    FVector Center = FVector::ZeroVector;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float Radius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float DamagePerSecond = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float Duration = 8.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede")
    float ElapsedTime = 0.0f;
};

// ── Stampede Controller Actor ────────────────────────────────────────────────
UCLASS(ClassGroup = "Crowd", meta = (DisplayName = "Crowd Stampede Controller"))
class TRANSPERSONALGAME_API ACrowd_StampedeController : public AActor
{
    GENERATED_BODY()

public:
    ACrowd_StampedeController();

    // ── Configuration ────────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede|Config")
    float TriggerRadius = 800.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede|Config")
    float MaxWaveRadius = 5000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede|Config")
    float PanicDecayRate = 0.15f;       // per second

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede|Config")
    float CalmDownTime = 12.0f;         // seconds before herd re-settles

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stampede|Config")
    bool bAutoTriggerOnPlayerProximity = true;

    // ── Runtime state ────────────────────────────────────────────────────────
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stampede|State")
    bool bStampedeActive = false;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stampede|State")
    float StampedeElapsedTime = 0.0f;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stampede|State")
    FVector StampedeDirection = FVector::ZeroVector;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stampede|State")
    TArray<FCrowd_StampedeWave> ActiveWaves;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Stampede|State")
    TArray<FCrowd_DangerZone> ActiveDangerZones;

    // ── Blueprint API ────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Stampede")
    void TriggerStampede(ECrowd_StampedeTrigger Trigger, FVector TriggerLocation, FVector FleeDirection);

    UFUNCTION(BlueprintCallable, Category = "Stampede")
    void StopStampede();

    UFUNCTION(BlueprintCallable, Category = "Stampede")
    bool IsLocationInDangerZone(FVector Location) const;

    UFUNCTION(BlueprintCallable, Category = "Stampede")
    float GetPanicIntensityAtLocation(FVector Location) const;

    UFUNCTION(BlueprintPure, Category = "Stampede")
    bool IsStampedeActive() const { return bStampedeActive; }

    UFUNCTION(CallInEditor, Category = "Stampede|Debug")
    void DEBUG_TriggerTestStampede();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

private:
    void UpdateWaves(float DeltaTime);
    void UpdateDangerZones(float DeltaTime);
    void SpawnDangerZoneAtLocation(FVector Location, float Radius, float DPS, float Duration);
    void NotifyNearbyHerds(FVector Origin, float PanicIntensity);

    float CalmDownTimer = 0.0f;
};
