#pragma once

#include "CoreMinimal.h"
#include "World/DinosaurBase.h"
#include "TRexCharacter.generated.h"

/**
 * ATRexCharacter — Tyrannosaurus Rex species implementation.
 *
 * Inherits from ADinosaurBase and overrides species-specific stats:
 * - High health (1200), massive damage (120), slow patrol / fast charge speed
 * - Territorial aggro radius 3500 units — largest of all species
 * - Roar ability on entering Combat state (Blueprint-extendable)
 * - Death triggers environmental reaction (screen shake, audio cue)
 *
 * Designed as a Blueprint-friendly base: all key events are BlueprintNativeEvent
 * so designers can add VFX/audio without touching C++.
 */
UCLASS(BlueprintType, Blueprintable, ClassGroup = "Dinosaurs", meta = (DisplayName = "T-Rex Character"))
class TRANSPERSONALGAME_API ATRexCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRexCharacter();

    // ── Species Overrides ────────────────────────────────────────────────────

    /** Called when T-Rex enters Combat state — triggers roar sequence */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TRex|Combat")
    void OnTRexRoar();
    virtual void OnTRexRoar_Implementation();

    /** Stomp attack — deals AoE damage in 400-unit radius around feet */
    UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "TRex|Combat")
    void PerformStompAttack();
    virtual void PerformStompAttack_Implementation();

    // ── T-Rex Specific Properties ────────────────────────────────────────────

    /** Radius of stomp AoE damage (cm) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompRadius = 400.0f;

    /** Stomp damage dealt to all actors in radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompDamage = 80.0f;

    /** Cooldown between stomp attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float StompCooldown = 6.0f;

    /** Camera shake class to trigger on stomp (assign in Blueprint) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    TSubclassOf<UCameraShakeBase> StompCameraShakeClass;

    /** Roar cooldown — prevents spam (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarCooldown = 12.0f;

    /** How far the roar intimidates prey (causes Fear stat increase) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarFearRadius = 5000.0f;

    /** Fear amount added to player SurvivalComponent when within roar radius */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Behavior")
    float RoarFearAmount = 35.0f;

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    /** Override state change to trigger roar on entering Combat */
    UFUNCTION()
    void HandleStateChanged(EEng_DinoState NewState);

    /** Override death to trigger environmental reaction */
    virtual void OnDinoDeath_Implementation() override;

private:
    float LastStompTime = 0.0f;
    float LastRoarTime = 0.0f;

    /** Apply fear to player characters within RoarFearRadius */
    void ApplyRoarFearToNearbyPlayers();

    /** Execute stomp AoE damage */
    void ExecuteStompAoE();
};
