// TRex.h
// Core Systems Programmer #03 — Transpersonal Game Studio
// Tyrannosaurus Rex — apex predator. High health, high damage, slow turn rate.
// Territorial ambush hunter. Charges prey within detection range.

#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "TRex.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATRex : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATRex();

    // ── TRex-specific stats ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Combat")
    float ChargeRange;

    UPROPERTY(BlueprintReadOnly, Category = "TRex|State")
    bool bIsCharging;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarRadius;       // Radius in which prey is frightened by roar

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TRex|Roar")
    float RoarCooldown;

    // ── Overrides ────────────────────────────────────────────────────────────
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // ── TRex abilities ───────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void StartCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "TRex|Combat")
    void StopCharge();

    UFUNCTION(BlueprintCallable, Category = "TRex|Roar")
    void PerformRoar();

    // ── Blueprint events ─────────────────────────────────────────────────────
    UFUNCTION(BlueprintNativeEvent, Category = "TRex|Events")
    void OnRoar();
    virtual void OnRoar_Implementation();

    UFUNCTION(BlueprintNativeEvent, Category = "TRex|Events")
    void OnChargeBegin(AActor* Target);
    virtual void OnChargeBegin_Implementation(AActor* Target);

private:
    float LastRoarTime;
    AActor* ChargeTarget;

    void UpdateCharge(float DeltaTime);
};
