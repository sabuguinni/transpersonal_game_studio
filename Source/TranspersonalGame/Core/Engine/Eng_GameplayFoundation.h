#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstanceSubsystem.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/GameModeBase.h"
#include "Components/ActorComponent.h"
#include "SharedTypes.h"
#include "Eng_GameplayFoundation.generated.h"

/**
 * CRITICAL GAMEPLAY FOUNDATION ARCHITECTURE
 * 
 * This system defines the core gameplay architecture that ALL other agents must follow.
 * No agent may create gameplay systems that violate these foundations.
 * 
 * MANDATE: Every gameplay system MUST inherit from or integrate with these base classes.
 */

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API UEng_GameplayFoundation : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UEng_GameplayFoundation();

	// === CORE GAMEPLAY ARCHITECTURE ===

	/** Initialize the gameplay foundation - called once at game start */
	UFUNCTION(BlueprintCallable, Category = "Gameplay Foundation")
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	/** Validate that all core gameplay systems are properly configured */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Gameplay Foundation")
	bool ValidateGameplayArchitecture();

	// === CHARACTER SYSTEM FOUNDATION ===

	/** Register a character class as valid for the game */
	UFUNCTION(BlueprintCallable, Category = "Character Foundation")
	void RegisterCharacterClass(TSubclassOf<ACharacter> CharacterClass);

	/** Get the primary player character class */
	UFUNCTION(BlueprintPure, Category = "Character Foundation")
	TSubclassOf<ACharacter> GetPrimaryCharacterClass() const { return PrimaryCharacterClass; }

	// === WORLD SYSTEM FOUNDATION ===

	/** Register a world system as active */
	UFUNCTION(BlueprintCallable, Category = "World Foundation")
	void RegisterWorldSystem(const FString& SystemName, UObject* SystemInstance);

	/** Get a registered world system by name */
	UFUNCTION(BlueprintCallable, Category = "World Foundation")
	UObject* GetWorldSystem(const FString& SystemName) const;

	// === SURVIVAL SYSTEM FOUNDATION ===

	/** Core survival stats that ALL characters must have */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival Foundation")
	FEng_SurvivalStats DefaultSurvivalStats;

	/** Register a survival component as valid */
	UFUNCTION(BlueprintCallable, Category = "Survival Foundation")
	void RegisterSurvivalComponent(TSubclassOf<UActorComponent> ComponentClass);

	// === DINOSAUR SYSTEM FOUNDATION ===

	/** Register a dinosaur species as valid for spawning */
	UFUNCTION(BlueprintCallable, Category = "Dinosaur Foundation")
	void RegisterDinosaurSpecies(const FString& SpeciesName, TSubclassOf<APawn> DinosaurClass);

	/** Get all registered dinosaur species */
	UFUNCTION(BlueprintPure, Category = "Dinosaur Foundation")
	TArray<FString> GetRegisteredDinosaurSpecies() const;

	// === PERFORMANCE FOUNDATION ===

	/** Maximum actors allowed in the world at once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Foundation")
	int32 MaxWorldActors = 8000;

	/** Maximum dinosaurs allowed in the world at once */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Performance Foundation")
	int32 MaxDinosaurs = 150;

	/** Check if we can spawn more actors */
	UFUNCTION(BlueprintPure, Category = "Performance Foundation")
	bool CanSpawnMoreActors() const;

	/** Check if we can spawn more dinosaurs */
	UFUNCTION(BlueprintPure, Category = "Performance Foundation")
	bool CanSpawnMoreDinosaurs() const;

	// === COMPILATION FOUNDATION ===

	/** Validate that all registered systems compile correctly */
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Compilation Foundation")
	bool ValidateSystemCompilation();

	/** Get compilation status of all systems */
	UFUNCTION(BlueprintPure, Category = "Compilation Foundation")
	TArray<FString> GetSystemCompilationStatus() const;

protected:
	// === INTERNAL ARCHITECTURE STATE ===

	/** Primary character class for the game */
	UPROPERTY()
	TSubclassOf<ACharacter> PrimaryCharacterClass;

	/** Registered character classes */
	UPROPERTY()
	TArray<TSubclassOf<ACharacter>> RegisteredCharacterClasses;

	/** Registered world systems */
	UPROPERTY()
	TMap<FString, UObject*> RegisteredWorldSystems;

	/** Registered survival components */
	UPROPERTY()
	TArray<TSubclassOf<UActorComponent>> RegisteredSurvivalComponents;

	/** Registered dinosaur species */
	UPROPERTY()
	TMap<FString, TSubclassOf<APawn>> RegisteredDinosaurSpecies;

	/** System compilation status cache */
	UPROPERTY()
	TMap<FString, bool> SystemCompilationStatus;

	// === VALIDATION METHODS ===

	/** Validate character system architecture */
	bool ValidateCharacterArchitecture();

	/** Validate world system architecture */
	bool ValidateWorldArchitecture();

	/** Validate survival system architecture */
	bool ValidateSurvivalArchitecture();

	/** Validate dinosaur system architecture */
	bool ValidateDinosaurArchitecture();

	/** Update system compilation status */
	void UpdateCompilationStatus();
};

/**
 * GAMEPLAY FOUNDATION STRUCTS
 * These are the core data structures that define gameplay architecture
 */

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_SurvivalStats
{
	GENERATED_BODY()

	/** Player health (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Health = 100.0f;

	/** Player hunger (0-100, 0 = starving) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Hunger = 100.0f;

	/** Player thirst (0-100, 0 = dehydrated) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Thirst = 100.0f;

	/** Player stamina (0-100) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Stamina = 100.0f;

	/** Player fear level (0-100, affects decision making) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Fear = 0.0f;

	/** Body temperature (affects survival) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Survival")
	float Temperature = 98.6f;

	FEng_SurvivalStats()
	{
		Health = 100.0f;
		Hunger = 100.0f;
		Thirst = 100.0f;
		Stamina = 100.0f;
		Fear = 0.0f;
		Temperature = 98.6f;
	}
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FEng_GameplaySystemInfo
{
	GENERATED_BODY()

	/** Name of the gameplay system */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
	FString SystemName;

	/** Whether the system is currently active */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
	bool bIsActive = false;

	/** Whether the system compiled successfully */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
	bool bCompiledSuccessfully = false;

	/** System priority (higher = more important) */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
	int32 Priority = 0;

	/** System dependencies */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "System Info")
	TArray<FString> Dependencies;

	FEng_GameplaySystemInfo()
	{
		SystemName = TEXT("Unknown");
		bIsActive = false;
		bCompiledSuccessfully = false;
		Priority = 0;
	}
};

#include "Eng_GameplayFoundation.generated.h"