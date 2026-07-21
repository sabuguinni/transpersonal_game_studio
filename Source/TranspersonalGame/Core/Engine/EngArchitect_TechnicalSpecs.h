#pragma once

#include "CoreMinimal.h"
#include "Engine/World.h"
#include "Components/ActorComponent.h"
#include "EngArchitect_TechnicalSpecs.generated.h"

/**
 * ENGINE ARCHITECT TECHNICAL SPECIFICATIONS v2.0
 * Core architectural patterns and constraints for all TranspersonalGame systems
 * Created by Agent #2 - Engine Architect
 */

// CORE PERFORMANCE CONSTRAINTS
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_PerformanceSpecs
{
    GENERATED_BODY()

    // Target framerate requirements
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TargetFPS_PC = 60;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 TargetFPS_Console = 30;
    
    // Actor limits per biome
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxActorsPerBiome = 2000;
    
    // Dinosaur simulation limits
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    int32 MaxSimultaneousDinosaurs = 50;
    
    // LOD distances
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float DinosaurLOD0_Distance = 1000.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float DinosaurLOD1_Distance = 2500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Performance")
    float DinosaurCull_Distance = 5000.0f;
};

// BIOME COORDINATE SYSTEM
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_BiomeCoordinates
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    FVector Savana = FVector(0.0f, 0.0f, 100.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    FVector Pantano = FVector(-50000.0f, -45000.0f, 50.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    FVector Floresta = FVector(-45000.0f, 40000.0f, 150.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    FVector Deserto = FVector(55000.0f, 0.0f, 200.0f);
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "World")
    FVector Montanha = FVector(40000.0f, 50000.0f, 800.0f);
};

// SURVIVAL SYSTEM ARCHITECTURE
USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalSpecs
{
    GENERATED_BODY()

    // Core survival stats ranges
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float MaxHealth = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float MaxHunger = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float MaxThirst = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float MaxStamina = 100.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float MaxFear = 100.0f;
    
    // Decay rates per second
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float HungerDecayRate = 0.1f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float ThirstDecayRate = 0.15f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Survival")
    float StaminaRegenRate = 2.0f;
};

// DINOSAUR BEHAVIOR ARCHITECTURE
UENUM(BlueprintType)
enum class EEng_DinosaurThreatLevel : uint8
{
    Passive     UMETA(DisplayName = "Passive"),
    Territorial UMETA(DisplayName = "Territorial"), 
    Aggressive  UMETA(DisplayName = "Aggressive"),
    Apex        UMETA(DisplayName = "Apex Predator")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_DinosaurSpecs
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    FString SpeciesName;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    EEng_DinosaurThreatLevel ThreatLevel = EEng_DinosaurThreatLevel::Passive;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    float DetectionRange = 1500.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    float AttackRange = 300.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    float MovementSpeed = 400.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    float Health = 200.0f;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Dinosaur")
    float Damage = 25.0f;
};

/**
 * Engine Architect Core Component
 * Enforces technical specifications across all game systems
 */
UCLASS(ClassGroup=(TranspersonalGame), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UEngArchitect_TechnicalSpecs : public UActorComponent
{
    GENERATED_BODY()

public:
    UEngArchitect_TechnicalSpecs();

protected:
    virtual void BeginPlay() override;

public:
    // Core specifications
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FEng_PerformanceSpecs PerformanceSpecs;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FEng_BiomeCoordinates BiomeCoordinates;
    
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Architecture")
    FEng_SurvivalSpecs SurvivalSpecs;

    // Validation functions
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool ValidatePerformanceMetrics();
    
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    FVector GetBiomeCenter(const FString& BiomeName);
    
    UFUNCTION(BlueprintCallable, Category = "Architecture")
    bool IsLocationInBiome(const FVector& Location, const FString& BiomeName);
    
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture")
    void ValidateWorldSetup();
    
    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Architecture") 
    void EnforceArchitecturalConstraints();
};