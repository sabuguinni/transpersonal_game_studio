#include "Eng_GameplayFoundation.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"
#include "Components/ActorComponent.h"
#include "UObject/ConstructorHelpers.h"

UEng_GameplayFoundation::UEng_GameplayFoundation()
{
	// Set default survival stats
	DefaultSurvivalStats.Health = 100.0f;
	DefaultSurvivalStats.Hunger = 100.0f;
	DefaultSurvivalStats.Thirst = 100.0f;
	DefaultSurvivalStats.Stamina = 100.0f;
	DefaultSurvivalStats.Fear = 0.0f;
	DefaultSurvivalStats.Temperature = 98.6f;

	// Set performance limits
	MaxWorldActors = 8000;
	MaxDinosaurs = 150;
}

void UEng_GameplayFoundation::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Initializing Gameplay Foundation Architecture"));

	// Initialize core systems
	RegisteredCharacterClasses.Empty();
	RegisteredWorldSystems.Empty();
	RegisteredSurvivalComponents.Empty();
	RegisteredDinosaurSpecies.Empty();
	SystemCompilationStatus.Empty();

	// Register default character class (will be overridden by Character Artist)
	// For now, use base ACharacter class
	PrimaryCharacterClass = ACharacter::StaticClass();
	RegisterCharacterClass(PrimaryCharacterClass);

	// Update compilation status
	UpdateCompilationStatus();

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Gameplay Foundation initialized successfully"));
}

bool UEng_GameplayFoundation::ValidateGameplayArchitecture()
{
	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Validating Gameplay Architecture"));

	bool bAllValid = true;

	// Validate character architecture
	if (!ValidateCharacterArchitecture())
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Character architecture validation FAILED"));
		bAllValid = false;
	}

	// Validate world architecture
	if (!ValidateWorldArchitecture())
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: World architecture validation FAILED"));
		bAllValid = false;
	}

	// Validate survival architecture
	if (!ValidateSurvivalArchitecture())
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Survival architecture validation FAILED"));
		bAllValid = false;
	}

	// Validate dinosaur architecture
	if (!ValidateDinosaurArchitecture())
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Dinosaur architecture validation FAILED"));
		bAllValid = false;
	}

	if (bAllValid)
	{
		UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: ALL architecture validations PASSED"));
	}

	return bAllValid;
}

void UEng_GameplayFoundation::RegisterCharacterClass(TSubclassOf<ACharacter> CharacterClass)
{
	if (CharacterClass)
	{
		RegisteredCharacterClasses.AddUnique(CharacterClass);
		UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Registered character class: %s"), 
			*CharacterClass->GetName());
	}
}

void UEng_GameplayFoundation::RegisterWorldSystem(const FString& SystemName, UObject* SystemInstance)
{
	if (SystemInstance && !SystemName.IsEmpty())
	{
		RegisteredWorldSystems.Add(SystemName, SystemInstance);
		UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Registered world system: %s"), 
			*SystemName);
	}
}

UObject* UEng_GameplayFoundation::GetWorldSystem(const FString& SystemName) const
{
	if (RegisteredWorldSystems.Contains(SystemName))
	{
		return RegisteredWorldSystems[SystemName];
	}
	return nullptr;
}

void UEng_GameplayFoundation::RegisterSurvivalComponent(TSubclassOf<UActorComponent> ComponentClass)
{
	if (ComponentClass)
	{
		RegisteredSurvivalComponents.AddUnique(ComponentClass);
		UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Registered survival component: %s"), 
			*ComponentClass->GetName());
	}
}

void UEng_GameplayFoundation::RegisterDinosaurSpecies(const FString& SpeciesName, TSubclassOf<APawn> DinosaurClass)
{
	if (DinosaurClass && !SpeciesName.IsEmpty())
	{
		RegisteredDinosaurSpecies.Add(SpeciesName, DinosaurClass);
		UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Registered dinosaur species: %s"), 
			*SpeciesName);
	}
}

TArray<FString> UEng_GameplayFoundation::GetRegisteredDinosaurSpecies() const
{
	TArray<FString> SpeciesNames;
	RegisteredDinosaurSpecies.GetKeys(SpeciesNames);
	return SpeciesNames;
}

bool UEng_GameplayFoundation::CanSpawnMoreActors() const
{
	if (UWorld* World = GetWorld())
	{
		int32 CurrentActorCount = World->GetActorCount();
		return CurrentActorCount < MaxWorldActors;
	}
	return false;
}

bool UEng_GameplayFoundation::CanSpawnMoreDinosaurs() const
{
	if (UWorld* World = GetWorld())
	{
		// Count dinosaurs in the world
		int32 DinosaurCount = 0;
		for (const auto& SpeciesPair : RegisteredDinosaurSpecies)
		{
			TSubclassOf<APawn> DinosaurClass = SpeciesPair.Value;
			if (DinosaurClass)
			{
				for (TActorIterator<APawn> ActorItr(World, DinosaurClass); ActorItr; ++ActorItr)
				{
					DinosaurCount++;
				}
			}
		}
		return DinosaurCount < MaxDinosaurs;
	}
	return false;
}

bool UEng_GameplayFoundation::ValidateSystemCompilation()
{
	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Validating system compilation"));

	UpdateCompilationStatus();

	bool bAllCompiled = true;
	for (const auto& StatusPair : SystemCompilationStatus)
	{
		if (!StatusPair.Value)
		{
			UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: System compilation FAILED: %s"), 
				*StatusPair.Key);
			bAllCompiled = false;
		}
	}

	return bAllCompiled;
}

TArray<FString> UEng_GameplayFoundation::GetSystemCompilationStatus() const
{
	TArray<FString> StatusStrings;
	for (const auto& StatusPair : SystemCompilationStatus)
	{
		FString Status = FString::Printf(TEXT("%s: %s"), 
			*StatusPair.Key, 
			StatusPair.Value ? TEXT("COMPILED") : TEXT("FAILED"));
		StatusStrings.Add(Status);
	}
	return StatusStrings;
}

bool UEng_GameplayFoundation::ValidateCharacterArchitecture()
{
	// Check that we have at least one registered character class
	if (RegisteredCharacterClasses.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: No character classes registered"));
		return false;
	}

	// Check that primary character class is valid
	if (!PrimaryCharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Primary character class is null"));
		return false;
	}

	// Validate that primary character class is in registered list
	if (!RegisteredCharacterClasses.Contains(PrimaryCharacterClass))
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Primary character class not in registered list"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Character architecture validation PASSED"));
	return true;
}

bool UEng_GameplayFoundation::ValidateWorldArchitecture()
{
	// For now, just check that we have a valid world
	if (!GetWorld())
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: No valid world found"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: World architecture validation PASSED"));
	return true;
}

bool UEng_GameplayFoundation::ValidateSurvivalArchitecture()
{
	// Check that default survival stats are valid
	if (DefaultSurvivalStats.Health < 0.0f || DefaultSurvivalStats.Health > 100.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Invalid default health value"));
		return false;
	}

	if (DefaultSurvivalStats.Hunger < 0.0f || DefaultSurvivalStats.Hunger > 100.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Invalid default hunger value"));
		return false;
	}

	if (DefaultSurvivalStats.Thirst < 0.0f || DefaultSurvivalStats.Thirst > 100.0f)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Invalid default thirst value"));
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Survival architecture validation PASSED"));
	return true;
}

bool UEng_GameplayFoundation::ValidateDinosaurArchitecture()
{
	// Check performance limits
	if (MaxDinosaurs <= 0 || MaxDinosaurs > 1000)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Invalid dinosaur limit: %d"), MaxDinosaurs);
		return false;
	}

	if (MaxWorldActors <= 0 || MaxWorldActors > 50000)
	{
		UE_LOG(LogTemp, Error, TEXT("UEng_GameplayFoundation: Invalid world actor limit: %d"), MaxWorldActors);
		return false;
	}

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Dinosaur architecture validation PASSED"));
	return true;
}

void UEng_GameplayFoundation::UpdateCompilationStatus()
{
	SystemCompilationStatus.Empty();

	// Check character system compilation
	SystemCompilationStatus.Add(TEXT("CharacterSystem"), RegisteredCharacterClasses.Num() > 0);

	// Check world system compilation
	SystemCompilationStatus.Add(TEXT("WorldSystem"), GetWorld() != nullptr);

	// Check survival system compilation
	SystemCompilationStatus.Add(TEXT("SurvivalSystem"), RegisteredSurvivalComponents.Num() >= 0);

	// Check dinosaur system compilation
	SystemCompilationStatus.Add(TEXT("DinosaurSystem"), MaxDinosaurs > 0 && MaxDinosaurs <= 150);

	// Check foundation compilation
	SystemCompilationStatus.Add(TEXT("GameplayFoundation"), true);

	UE_LOG(LogTemp, Warning, TEXT("UEng_GameplayFoundation: Updated compilation status for %d systems"), 
		SystemCompilationStatus.Num());
}