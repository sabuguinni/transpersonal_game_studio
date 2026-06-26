#pragma once

#include "CoreMinimal.h"
#include "Dinosaurs/DinosaurBase.h"
#include "VelociraptorCharacter.generated.h"

/**
 * VelociraptorCharacter — Pack-hunting dinosaur species
 * Coordinates with nearby raptor allies for flanking attacks.
 * Inherits from ADinosaurBase (ACharacter chain).
 */
UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AVelociraptorCharacter : public ADinosaurBase
{
    GENERATED_BODY()

public:
    AVelociraptorCharacter();

    virtual void BeginPlay() override;
    virtual void Tick(float DeltaTime) override;

    // --- Pack Hunting ---

    /** Radius within which this raptor looks for pack allies */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    float PackSearchRadius;

    /** Maximum number of pack members to coordinate with */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Pack")
    int32 MaxPackSize;

    /** Current pack allies (weak refs — no GC ownership) */
    UPROPERTY(BlueprintReadOnly, Category = "Velociraptor|Pack")
    TArray<AVelociraptorCharacter*> PackAllies;

    /** True if this raptor is the pack Alpha */
    UPROPERTY(BlueprintReadOnly, Category = "Velociraptor|Pack")
    bool bIsAlpha;

    /** Find nearby raptor allies and populate PackAllies */
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void ScanForPackAllies();

    /** Coordinate a flanking attack — Alpha calls this, allies respond */
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void InitiatePackAttack(AActor* Target);

    /** Called on non-Alpha raptors by the Alpha to flank from a given direction */
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Pack")
    void RespondToPackAttack(AActor* Target, FVector FlankDirection);

    // --- Combat Abilities ---

    /** Pounce leap attack — launches raptor at target */
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    void PerformPounce(AActor* Target);

    /** Slashing claw attack — fast melee at close range */
    UFUNCTION(BlueprintCallable, Category = "Velociraptor|Combat")
    void PerformClawSlash();

    /** Cooldown between pounce attacks (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float PounceCooldown;

    /** Damage dealt by claw slash */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float ClawDamage;

    /** Pounce launch velocity multiplier */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|Combat")
    float PounceVelocityScale;

    // --- Species Stats ---

    /** Pack scan interval (seconds) */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Velociraptor|AI")
    float PackScanInterval;

protected:
    /** Initialize Velociraptor-specific stats */
    void InitVelociraptorStats();

    /** Timer handle for periodic pack scanning */
    FTimerHandle PackScanTimerHandle;

    /** Timer handle for pounce cooldown */
    FTimerHandle PounceCooldownTimerHandle;

    /** Whether pounce is currently on cooldown */
    bool bPounceCoolingDown;
};
