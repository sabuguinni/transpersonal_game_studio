#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "TRexBehavior.h"
#include "NPCBehaviorRegistry.generated.h"

/**
 * Registry of all NPC behavior configurations for the prehistoric survival world.
 * Provides factory methods and default configs for each dinosaur/NPC species.
 * Agent #11 — NPC Behavior Agent
 */

UENUM(BlueprintType)
enum class ENPC_DinosaurSpecies : uint8
{
    TRex            UMETA(DisplayName = "Tyrannosaurus Rex"),
    Raptor          UMETA(DisplayName = "Velociraptor"),
    Brachiosaurus   UMETA(DisplayName = "Brachiosaurus"),
    Triceratops     UMETA(DisplayName = "Triceratops"),
    Pterodactyl     UMETA(DisplayName = "Pterodactyl"),
    Stegosaurus     UMETA(DisplayName = "Stegosaurus")
};

UENUM(BlueprintType)
enum class ENPC_ThreatLevel : uint8
{
    Passive         UMETA(DisplayName = "Passive"),
    Cautious        UMETA(DisplayName = "Cautious"),
    Territorial     UMETA(DisplayName = "Territorial"),
    Aggressive      UMETA(DisplayName = "Aggressive"),
    Apex            UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct FNPC_DinosaurProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
    ENPC_DinosaurSpecies Species = ENPC_DinosaurSpecies::TRex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
    ENPC_ThreatLevel ThreatLevel = ENPC_ThreatLevel::Apex;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Profile")
    FString DisplayName = TEXT("Unknown Dinosaur");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float MaxHealth = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float DetectionRange = 2000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float AttackRange = 300.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float AttackDamage = 50.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Stats")
    float MoveSpeed = 400.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    bool bIsPackHunter = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    bool bIsNocturnal = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float PatrolRadius = 3000.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "NPC|Behavior")
    float FleeHealthThreshold = 0.2f;
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UNPCBehaviorRegistry : public UObject
{
    GENERATED_BODY()

public:
    UNPCBehaviorRegistry();

    // Get default profile for a species
    UFUNCTION(BlueprintCallable, Category = "NPC|Registry")
    static FNPC_DinosaurProfile GetDefaultProfile(ENPC_DinosaurSpecies Species);

    // Get T-Rex config from profile
    UFUNCTION(BlueprintCallable, Category = "NPC|Registry")
    static FNPC_TRexConfig GetTRexConfig();

    // Get all registered species
    UFUNCTION(BlueprintCallable, Category = "NPC|Registry")
    static TArray<ENPC_DinosaurSpecies> GetAllSpecies();

    // Check if species is predator
    UFUNCTION(BlueprintPure, Category = "NPC|Registry")
    static bool IsPredator(ENPC_DinosaurSpecies Species);

    // Check if species is herbivore
    UFUNCTION(BlueprintPure, Category = "NPC|Registry")
    static bool IsHerbivore(ENPC_DinosaurSpecies Species);

    // Get threat level description
    UFUNCTION(BlueprintPure, Category = "NPC|Registry")
    static FString GetThreatDescription(ENPC_ThreatLevel ThreatLevel);
};
