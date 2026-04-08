#pragma once

#include "CoreMinimal.h"
#include "Engine/Engine.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/DataTable.h"
#include "CharacterSystem.generated.h"

// Enum para tipos de personagem
UENUM(BlueprintType)
enum class ECharacterType : uint8
{
    Protagonist     UMETA(DisplayName = "Protagonist"),
    TribalLeader    UMETA(DisplayName = "Tribal Leader"), 
    TribalMember    UMETA(DisplayName = "Tribal Member"),
    Shaman          UMETA(DisplayName = "Shaman"),
    Hunter          UMETA(DisplayName = "Hunter"),
    Gatherer        UMETA(DisplayName = "Gatherer"),
    Child           UMETA(DisplayName = "Child"),
    Elder           UMETA(DisplayName = "Elder")
};

// Enum para etnias/grupos tribais
UENUM(BlueprintType)
enum class ETribalGroup : uint8
{
    RiverTribe      UMETA(DisplayName = "River Tribe"),
    MountainTribe   UMETA(DisplayName = "Mountain Tribe"),
    ForestTribe     UMETA(DisplayName = "Forest Tribe"),
    PlainsTribe     UMETA(DisplayName = "Plains Tribe"),
    CoastalTribe    UMETA(DisplayName = "Coastal Tribe")
};

// Struct para variações físicas
USTRUCT(BlueprintType)
struct FPhysicalVariation
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Height = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float Weight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    float MuscleDefinition = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor SkinTone = FLinearColor(0.8f, 0.6f, 0.4f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor HairColor = FLinearColor(0.2f, 0.1f, 0.05f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FLinearColor EyeColor = FLinearColor(0.3f, 0.2f, 0.1f, 1.0f);
};

// Struct para dados de personagem
USTRUCT(BlueprintType)
struct FCharacterData : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString CharacterName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ECharacterType CharacterType;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    ETribalGroup TribalGroup;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FPhysicalVariation PhysicalTraits;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TSoftObjectPtr<class USkeletalMesh> MetaHumanMesh;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    TArray<TSoftObjectPtr<class UMaterialInterface>> ClothingMaterials;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    FString Biography;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite)
    bool bIsUnique = false; // Para personagens nomeados importantes
};

// Component para gerenciar aparência de personagens
UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class TRANSPERSONALGAME_API UCharacterAppearanceComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCharacterAppearanceComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character")
    FCharacterData CharacterData;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    class USkeletalMeshComponent* MetaHumanMesh;

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyCharacterData(const FCharacterData& NewCharacterData);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void GenerateRandomAppearance(ETribalGroup Tribe, ECharacterType Type);

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyPhysicalVariations();

    UFUNCTION(BlueprintCallable, Category = "Character")
    void ApplyClothing();

protected:
    virtual void BeginPlay() override;

private:
    void SetupMetaHumanParameters();
    void ApplySkinTone(FLinearColor SkinColor);
    void ApplyHairColor(FLinearColor HairColor);
    void ApplyEyeColor(FLinearColor EyeColor);
    void ApplyBodyShape(float Height, float Weight, float Muscle);
};

// Subsystem para geração procedural de personagens
UCLASS()
class TRANSPERSONALGAME_API UCharacterGenerationSubsystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual void Initialize(FSubsystemCollectionBase& Collection) override;

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    FCharacterData GenerateRandomCharacter(ETribalGroup Tribe, ECharacterType Type);

    UFUNCTION(BlueprintCallable, Category = "Character Generation")
    TArray<FCharacterData> GenerateTribalPopulation(ETribalGroup Tribe, int32 PopulationSize);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Database")
    class UDataTable* CharacterDatabase;

protected:
    // Pools de variação genética por tribo
    TMap<ETribalGroup, TArray<FPhysicalVariation>> TribalGenePool;

    void InitializeTribalGenePools();
    FPhysicalVariation BlendPhysicalTraits(const FPhysicalVariation& Parent1, const FPhysicalVariation& Parent2);
    FLinearColor GenerateTribalSkinTone(ETribalGroup Tribe);
    FLinearColor GenerateTribalHairColor(ETribalGroup Tribe);
    FLinearColor GenerateTribalEyeColor(ETribalGroup Tribe);
};