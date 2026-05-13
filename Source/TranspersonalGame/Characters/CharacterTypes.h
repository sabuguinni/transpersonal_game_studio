#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "CharacterTypes.generated.h"

UENUM(BlueprintType)
enum class EChar_TribalRole : uint8
{
    Hunter      UMETA(DisplayName = "Hunter"),
    Gatherer    UMETA(DisplayName = "Gatherer"),
    Warrior     UMETA(DisplayName = "Warrior"),
    Elder       UMETA(DisplayName = "Elder"),
    Child       UMETA(DisplayName = "Child"),
    Shaman      UMETA(DisplayName = "Shaman"),
    Crafter     UMETA(DisplayName = "Crafter")
};

UENUM(BlueprintType)
enum class EChar_BodyType : uint8
{
    Lean        UMETA(DisplayName = "Lean"),
    Athletic    UMETA(DisplayName = "Athletic"),
    Muscular    UMETA(DisplayName = "Muscular"),
    Stocky      UMETA(DisplayName = "Stocky"),
    Elder       UMETA(DisplayName = "Elder")
};

UENUM(BlueprintType)
enum class EChar_SkinTone : uint8
{
    Fair        UMETA(DisplayName = "Fair"),
    Olive       UMETA(DisplayName = "Olive"),
    Tan         UMETA(DisplayName = "Tan"),
    Dark        UMETA(DisplayName = "Dark"),
    Weathered   UMETA(DisplayName = "Weathered")
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_TribalAppearance
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_TribalRole Role = EChar_TribalRole::Hunter;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_BodyType BodyType = EChar_BodyType::Athletic;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    EChar_SkinTone SkinTone = EChar_SkinTone::Tan;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString HairStyle = TEXT("Braided");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString FacialHair = TEXT("None");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> Scars;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> Tattoos;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    FString ClothingStyle = TEXT("Leather_Basic");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Appearance")
    TArray<FString> Accessories;

    FChar_TribalAppearance()
    {
        Scars.Add(TEXT("Face_Scar_01"));
        Tattoos.Add(TEXT("Tribal_Marks_Basic"));
        Accessories.Add(TEXT("Bone_Necklace"));
    }
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_MetaHumanConfig : public FTableRowBase
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString MetaHumanAssetPath;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FChar_TribalAppearance Appearance;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float Height = 175.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    float Weight = 70.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    int32 Age = 25;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    bool bIsMale = true;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString CharacterName = TEXT("Tribal_Hunter");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MetaHuman")
    FString Biography = TEXT("A skilled hunter who has survived the dangers of the Cretaceous period.");
};

USTRUCT(BlueprintType)
struct TRANSPERSONALGAME_API FChar_ClothingSet
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString SetName = TEXT("Basic_Leather");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString ChestPiece = TEXT("Leather_Vest");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString LegPiece = TEXT("Leather_Pants");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString FootPiece = TEXT("Fur_Boots");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FString HeadPiece = TEXT("None");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    TArray<FString> Accessories;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor PrimaryColor = FLinearColor(0.4f, 0.3f, 0.2f, 1.0f);

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Clothing")
    FLinearColor SecondaryColor = FLinearColor(0.2f, 0.15f, 0.1f, 1.0f);
};