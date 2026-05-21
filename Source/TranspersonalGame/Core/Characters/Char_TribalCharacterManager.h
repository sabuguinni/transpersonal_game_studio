#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "SharedTypes.h"
#include "Char_TribalCharacterManager.generated.h"

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TSoftObjectPtr<USkeletalMesh> CharacterMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FVector DefaultSpawnLocation;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    EBiomeType PreferredBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float HealthPoints;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    float MovementSpeed;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    TArray<FString> EquippedTools;

    FChar_TribalCharacterData()
    {
        CharacterName = TEXT("Unknown Tribal");
        DefaultSpawnLocation = FVector::ZeroVector;
        PreferredBiome = EBiomeType::Savana;
        HealthPoints = 100.0f;
        MovementSpeed = 300.0f;
    }
};

UCLASS(BlueprintType, Blueprintable)
class TRANSPERSONALGAME_API AChar_TribalCharacterManager : public AActor
{
    GENERATED_BODY()

public:
    AChar_TribalCharacterManager();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Management")
    UDataTable* TribalCharacterDataTable;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Management")
    TArray<AActor*> SpawnedTribalCharacters;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Management")
    int32 MaxCharactersPerBiome;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character Management")
    float SpawnRadius;

public:
    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SpawnTribalCharactersInBiome(EBiomeType Biome, int32 Count);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void SpawnSpecificTribalCharacter(const FString& CharacterName, FVector Location);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    TArray<AActor*> GetTribalCharactersInBiome(EBiomeType Biome);

    UFUNCTION(BlueprintCallable, Category = "Character Management")
    void RemoveAllTribalCharacters();

    UFUNCTION(BlueprintCallable, CallInEditor, Category = "Character Management")
    void PopulateAllBiomesWithCharacters();

protected:
    UFUNCTION()
    AActor* CreateTribalCharacterActor(const FChar_TribalCharacterData& CharacterData, FVector SpawnLocation);

    UFUNCTION()
    FVector GetRandomLocationInBiome(EBiomeType Biome);

    UFUNCTION()
    bool IsValidSpawnLocation(FVector Location);
};