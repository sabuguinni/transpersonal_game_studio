#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Materials/MaterialInterface.h"
#include "SharedTypes.h"
#include "Char_TribalCharacterSpawner.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalArchetype : uint8
{
    Explorer        UMETA(DisplayName = "Explorer/Paleontologist"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Elder           UMETA(DisplayName = "Tribal Elder"),
    Hunter          UMETA(DisplayName = "Young Hunter"),
    Child           UMETA(DisplayName = "Tribal Child")
};

USTRUCT(BlueprintType)
struct FChar_TribalCharacterData
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EChar_TribalArchetype Archetype = EChar_TribalArchetype::Explorer;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName = TEXT("Unnamed Tribal");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float Age = 25.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString ClothingDescription = TEXT("Woven plant fiber and leather");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> EquipmentList;

    FChar_TribalCharacterData()
    {
        EquipmentList.Add(TEXT("Stone Tools"));
        EquipmentList.Add(TEXT("Gathering Basket"));
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacterSpawner : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalCharacterSpawner();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* CharacterMesh;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    UStaticMeshComponent* EquipmentMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Data")
    FChar_TribalCharacterData CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    int32 MaxCharactersToSpawn = 5;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Spawning")
    float SpawnRadius = 500.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ExplorerMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* GathererMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ElderMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* HunterMaterial;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Materials")
    UMaterialInterface* ChildMaterial;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Spawning")
    void SpawnTribalCharacter(EChar_TribalArchetype ArchetypeToSpawn, FVector SpawnLocation);

    UFUNCTION(BlueprintCallable, Category = "Character Spawning")
    void SpawnRandomTribalGroup();

    UFUNCTION(BlueprintCallable, Category = "Character Spawning")
    FChar_TribalCharacterData GenerateRandomCharacterData(EChar_TribalArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Spawning")
    UMaterialInterface* GetMaterialForArchetype(EChar_TribalArchetype Archetype);

    UFUNCTION(BlueprintCallable, Category = "Character Spawning")
    FString GetArchetypeDescription(EChar_TribalArchetype Archetype);

private:
    void SetupDefaultMeshes();
    void ApplyCharacterMaterial(EChar_TribalArchetype Archetype);
    FVector GetRandomSpawnLocation();
};