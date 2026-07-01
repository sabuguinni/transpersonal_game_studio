#pragma once

#include "CoreMinimal.h"
#include "DinosaurBase.h"
#include "TriceratopsDinosaur.generated.h"

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API ATriceratopsDinosaur : public ADinosaurBase
{
    GENERATED_BODY()

public:
    ATriceratopsDinosaur();

protected:
    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

public:
    // ── HERD PROPERTIES ─────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Herd")
    int32 HerdID;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Herd")
    float HerdProtectionRadius;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Herd")
    int32 MaxHerdSize;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Herd")
    bool bIsHerdLeader;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Herd")
    bool bIsProtectingJuvenile;

    // ── HORN CHARGE ATTACK ──────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Combat")
    float HornChargeRange;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Combat")
    float HornChargeDamage;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Combat")
    float HornChargeCooldown;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Combat")
    float HornChargeSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Combat")
    float HornChargeKnockbackForce;

    UPROPERTY(BlueprintReadOnly, Category = "Triceratops|Combat",
        meta = (AllowPrivateAccess = "true"))
    bool bIsCharging;

    UPROPERTY(BlueprintReadOnly, Category = "Triceratops|Combat",
        meta = (AllowPrivateAccess = "true"))
    float CurrentChargeCooldown;

    // ── DEFENSIVE STANCE ────────────────────────────────────────────────────
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Defense")
    float FrillArmorMultiplier;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Defense")
    bool bIsInDefensiveStance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Triceratops|Defense")
    float DefensiveStanceDamageReduction;

    // ── FUNCTIONS ────────────────────────────────────────────────────────────
    UFUNCTION(BlueprintCallable, Category = "Triceratops|Combat")
    void ExecuteHornCharge(AActor* Target);

    UFUNCTION(BlueprintCallable, Category = "Triceratops|Herd")
    void SignalHerdDefense(AActor* Threat);

    UFUNCTION(BlueprintCallable, Category = "Triceratops|Defense")
    void EnterDefensiveStance();

    UFUNCTION(BlueprintCallable, Category = "Triceratops|Defense")
    void ExitDefensiveStance();

    UFUNCTION(BlueprintPure, Category = "Triceratops|Combat")
    bool CanCharge() const;

    UFUNCTION(BlueprintPure, Category = "Triceratops|Herd")
    bool IsHerdLeader() const;

private:
    void UpdateChargeCooldown(float DeltaTime);
    void FormHerdCircle(AActor* Threat);
};
