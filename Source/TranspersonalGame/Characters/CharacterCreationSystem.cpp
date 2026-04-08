#include "CharacterCreationSystem.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/KismetStringLibrary.h"

UCharacterCreationSystem::UCharacterCreationSystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UCharacterCreationSystem::BeginPlay()
{
    Super::BeginPlay();
    
    // Inicializar sistema se necessário
    if (CharacterDefinitionsTable)
    {
        UE_LOG(LogTemp, Warning, TEXT("Character Creation System initialized with DataTable"));
    }
}

FCharacterDefinition UCharacterCreationSystem::GenerateUniqueCharacter(ECharacterArchetype Archetype, bool bEnsureUniqueness)
{
    FCharacterDefinition NewCharacter;
    
    // Configurar arquétipo
    NewCharacter.Archetype = Archetype;
    
    // Gerar características aleatórias
    NewCharacter.Ethnicity = static_cast<EEthnicity>(FMath::RandRange(0, 6));
    NewCharacter.AgeRange = static_cast<EAgeRange>(FMath::RandRange(0, 3));
    NewCharacter.bIsMale = FMath::RandBool();
    
    // Gerar traits físicos únicos
    NewCharacter.PhysicalTraits = GenerateRandomPhysicalTraits(NewCharacter.Ethnicity, NewCharacter.AgeRange, NewCharacter.bIsMale);
    
    // Selecionar outfit apropriado
    NewCharacter.CurrentOutfit = SelectAppropriateOutfit(Archetype, NewCharacter.AgeRange);
    
    // Gerar ID único
    NewCharacter.UniqueID = GenerateUniqueCharacterID();
    
    // Gerar nome baseado na etnia e gênero
    switch (NewCharacter.Ethnicity)
    {
        case EEthnicity::European:
            NewCharacter.CharacterName = NewCharacter.bIsMale ? 
                TEXT("Marcus, Erik, James, David, Thomas") : 
                TEXT("Elena, Sarah, Emma, Anna, Sofia");
            break;
        case EEthnicity::African:
            NewCharacter.CharacterName = NewCharacter.bIsMale ? 
                TEXT("Kwame, Amari, Kofi, Jengo, Tau") : 
                TEXT("Amina, Zara, Kesi, Nia, Imara");
            break;
        case EEthnicity::Asian:
            NewCharacter.CharacterName = NewCharacter.bIsMale ? 
                TEXT("Hiroshi, Wei, Raj, Kenji, Arjun") : 
                TEXT("Yuki, Li, Priya, Mei, Sakura");
            break;
        case EEthnicity::LatinAmerican:
            NewCharacter.CharacterName = NewCharacter.bIsMale ? 
                TEXT("Carlos, Diego, Miguel, Rafael, Alejandro") : 
                TEXT("Maria, Carmen, Isabella, Sofia, Lucia");
            break;
        case EEthnicity::MiddleEastern:
            NewCharacter.CharacterName = NewCharacter.bIsMale ? 
                TEXT("Omar, Hassan, Khalil, Samir, Tariq") : 
                TEXT("Layla, Fatima, Yasmin, Nadia, Amira");
            break;
        case EEthnicity::Indigenous:
            NewCharacter.CharacterName = NewCharacter.bIsMale ? 
                TEXT("Aiyana, Takoda, Koa, Naalnish, Ezhno") : 
                TEXT("Aiyana, Kachina, Nayeli, Suki, Tala");
            break;
        default:
            NewCharacter.CharacterName = TEXT("Unknown");
            break;
    }
    
    // Gerar background story baseado no arquétipo
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            NewCharacter.BackgroundStory = TEXT("Paleontologista experiente que foi transportado no tempo ao tocar uma gema misteriosa durante uma escavação.");
            break;
        case ECharacterArchetype::Survivor:
            NewCharacter.BackgroundStory = TEXT("Sobrevivente que chegou ao mundo pré-histórico por circunstâncias desconhecidas e aprendeu a viver entre os dinossauros.");
            break;
        case ECharacterArchetype::Researcher:
            NewCharacter.BackgroundStory = TEXT("Cientista ou acadêmico que se encontra perdido no tempo, usando conhecimento para sobreviver.");
            break;
        case ECharacterArchetype::Explorer:
            NewCharacter.BackgroundStory = TEXT("Aventureiro experiente que se adapta rapidamente ao ambiente hostil do período jurássico.");
            break;
        case ECharacterArchetype::Tribal:
            NewCharacter.BackgroundStory = TEXT("Pessoa que formou ou se juntou a uma pequena comunidade de sobreviventes no mundo pré-histórico.");
            break;
        case ECharacterArchetype::Wanderer:
            NewCharacter.BackgroundStory = TEXT("Indivíduo solitário que vaga pelo mundo pré-histórico, evitando contato com outros humanos.");
            break;
    }
    
    // Verificar unicidade se solicitado
    if (bEnsureUniqueness)
    {
        int32 Attempts = 0;
        while (!IsCharacterUnique(NewCharacter) && Attempts < 100)
        {
            // Regenerar algumas características para garantir unicidade
            NewCharacter.PhysicalTraits = GenerateRandomPhysicalTraits(NewCharacter.Ethnicity, NewCharacter.AgeRange, NewCharacter.bIsMale);
            NewCharacter.UniqueID = GenerateUniqueCharacterID();
            Attempts++;
        }
    }
    
    // Registrar personagem
    RegisterCharacter(NewCharacter);
    
    return NewCharacter;
}

FCharacterDefinition UCharacterCreationSystem::CreateCharacterVariation(const FCharacterDefinition& BaseCharacter)
{
    FCharacterDefinition Variation = BaseCharacter;
    
    // Aplicar pequenas variações mantendo a base
    Variation.PhysicalTraits.Height += FMath::RandRange(-0.1f, 0.1f);
    Variation.PhysicalTraits.Weight += FMath::RandRange(-0.1f, 0.1f);
    
    // Pequenas variações na cor da pele
    Variation.PhysicalTraits.SkinTone.R += FMath::RandRange(-0.05f, 0.05f);
    Variation.PhysicalTraits.SkinTone.G += FMath::RandRange(-0.05f, 0.05f);
    Variation.PhysicalTraits.SkinTone.B += FMath::RandRange(-0.05f, 0.05f);
    
    // Novo ID único
    Variation.UniqueID = GenerateUniqueCharacterID();
    
    // Adicionar variação no nome
    Variation.CharacterName += TEXT(" (Var)");
    
    return Variation;
}

bool UCharacterCreationSystem::IsCharacterUnique(const FCharacterDefinition& Character)
{
    for (const FCharacterDefinition& ExistingCharacter : GeneratedCharacters)
    {
        // Verificar se é muito similar a um personagem existente
        if (ExistingCharacter.Ethnicity == Character.Ethnicity &&
            ExistingCharacter.AgeRange == Character.AgeRange &&
            ExistingCharacter.bIsMale == Character.bIsMale &&
            FMath::Abs(ExistingCharacter.PhysicalTraits.Height - Character.PhysicalTraits.Height) < 0.05f &&
            FMath::Abs(ExistingCharacter.PhysicalTraits.Weight - Character.PhysicalTraits.Weight) < 0.05f)
        {
            return false;
        }
    }
    return true;
}

FCharacterDefinition UCharacterCreationSystem::GetCharacterByID(const FString& UniqueID)
{
    for (const FCharacterDefinition& Character : GeneratedCharacters)
    {
        if (Character.UniqueID == UniqueID)
        {
            return Character;
        }
    }
    return FCharacterDefinition();
}

void UCharacterCreationSystem::RegisterCharacter(const FCharacterDefinition& Character)
{
    GeneratedCharacters.Add(Character);
    UE_LOG(LogTemp, Warning, TEXT("Registered character: %s (ID: %s)"), *Character.CharacterName, *Character.UniqueID);
}

FString UCharacterCreationSystem::GenerateUniqueCharacterID()
{
    FString Timestamp = FString::FromInt(FDateTime::Now().ToUnixTimestamp());
    FString RandomSuffix = FString::FromInt(FMath::RandRange(1000, 9999));
    return FString::Printf(TEXT("CHAR_%s_%s"), *Timestamp, *RandomSuffix);
}

FPhysicalTraits UCharacterCreationSystem::GenerateRandomPhysicalTraits(EEthnicity Ethnicity, EAgeRange AgeRange, bool bIsMale)
{
    FPhysicalTraits Traits;
    
    // Altura baseada em gênero e idade
    float BaseHeight = bIsMale ? 1.0f : 0.93f; // Homens ligeiramente mais altos
    switch (AgeRange)
    {
        case EAgeRange::Young:
            BaseHeight += FMath::RandRange(-0.05f, 0.05f);
            break;
        case EAgeRange::Adult:
            BaseHeight += FMath::RandRange(-0.03f, 0.03f);
            break;
        case EAgeRange::Mature:
            BaseHeight += FMath::RandRange(-0.02f, 0.02f);
            break;
        case EAgeRange::Elder:
            BaseHeight -= FMath::RandRange(0.02f, 0.08f); // Ligeiramente mais baixos
            break;
    }
    Traits.Height = FMath::Clamp(BaseHeight, 0.8f, 1.2f);
    
    // Peso baseado em idade
    float BaseWeight = 1.0f;
    switch (AgeRange)
    {
        case EAgeRange::Young:
            BaseWeight = FMath::RandRange(0.85f, 1.05f);
            break;
        case EAgeRange::Adult:
            BaseWeight = FMath::RandRange(0.9f, 1.15f);
            break;
        case EAgeRange::Mature:
            BaseWeight = FMath::RandRange(0.95f, 1.25f);
            break;
        case EAgeRange::Elder:
            BaseWeight = FMath::RandRange(0.8f, 1.1f);
            break;
    }
    Traits.Weight = BaseWeight;
    
    // Tom de pele baseado na etnia
    switch (Ethnicity)
    {
        case EEthnicity::European:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.8f, 0.95f), FMath::RandRange(0.7f, 0.9f), FMath::RandRange(0.7f, 0.85f), 1.0f);
            break;
        case EEthnicity::African:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.2f, 0.6f), FMath::RandRange(0.15f, 0.45f), FMath::RandRange(0.1f, 0.35f), 1.0f);
            break;
        case EEthnicity::Asian:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.7f, 0.9f), FMath::RandRange(0.65f, 0.8f), FMath::RandRange(0.5f, 0.7f), 1.0f);
            break;
        case EEthnicity::LatinAmerican:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.6f, 0.85f), FMath::RandRange(0.5f, 0.75f), FMath::RandRange(0.4f, 0.65f), 1.0f);
            break;
        case EEthnicity::MiddleEastern:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.65f, 0.8f), FMath::RandRange(0.55f, 0.7f), FMath::RandRange(0.45f, 0.6f), 1.0f);
            break;
        case EEthnicity::Indigenous:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.5f, 0.75f), FMath::RandRange(0.4f, 0.65f), FMath::RandRange(0.3f, 0.55f), 1.0f);
            break;
        default:
            Traits.SkinTone = FLinearColor(FMath::RandRange(0.4f, 0.9f), FMath::RandRange(0.3f, 0.8f), FMath::RandRange(0.2f, 0.7f), 1.0f);
            break;
    }
    
    // Cor do cabelo
    TArray<FLinearColor> HairColors = {
        FLinearColor::Black,
        FLinearColor(0.3f, 0.15f, 0.05f, 1.0f), // Castanho escuro
        FLinearColor(0.5f, 0.3f, 0.1f, 1.0f),   // Castanho
        FLinearColor(0.8f, 0.6f, 0.2f, 1.0f),   // Louro escuro
        FLinearColor(0.9f, 0.8f, 0.4f, 1.0f),   // Louro
        FLinearColor(0.6f, 0.2f, 0.1f, 1.0f),   // Ruivo
        FLinearColor(0.7f, 0.7f, 0.7f, 1.0f)    // Grisalho (para idosos)
    };
    
    if (AgeRange == EAgeRange::Elder)
    {
        // Idosos têm maior chance de cabelo grisalho
        Traits.HairColor = FMath::RandBool() ? HairColors.Last() : HairColors[FMath::RandRange(0, HairColors.Num() - 2)];
    }
    else
    {
        Traits.HairColor = HairColors[FMath::RandRange(0, HairColors.Num() - 2)];
    }
    
    // Cor dos olhos
    TArray<FLinearColor> EyeColors = {
        FLinearColor(0.3f, 0.15f, 0.05f, 1.0f), // Castanho
        FLinearColor(0.1f, 0.3f, 0.6f, 1.0f),   // Azul
        FLinearColor(0.2f, 0.5f, 0.2f, 1.0f),   // Verde
        FLinearColor(0.4f, 0.3f, 0.1f, 1.0f),   // Avelã
        FLinearColor(0.15f, 0.1f, 0.05f, 1.0f), // Castanho escuro
        FLinearColor(0.5f, 0.5f, 0.5f, 1.0f)    // Cinza
    };
    Traits.EyeColor = EyeColors[FMath::RandRange(0, EyeColors.Num() - 1)];
    
    // Características únicas ocasionais
    if (FMath::RandRange(0, 100) < 15) // 15% de chance
    {
        TArray<FString> UniqueFeatures = {
            TEXT("Cicatriz no rosto"),
            TEXT("Marca de nascença"),
            TEXT("Tatuagem tribal"),
            TEXT("Piercing"),
            TEXT("Dente de ouro"),
            TEXT("Olhos de cores diferentes"),
            TEXT("Cabelo com mechas brancas"),
            TEXT("Sardas proeminentes")
        };
        Traits.UniqueFeature = UniqueFeatures[FMath::RandRange(0, UniqueFeatures.Num() - 1)];
    }
    
    return Traits;
}

FCharacterOutfit UCharacterCreationSystem::SelectAppropriateOutfit(ECharacterArchetype Archetype, EAgeRange AgeRange)
{
    FCharacterOutfit Outfit;
    
    switch (Archetype)
    {
        case ECharacterArchetype::Protagonist:
            Outfit.OutfitName = TEXT("Roupa de Campo de Paleontólogo");
            Outfit.Description = TEXT("Roupas práticas de trabalho de campo, ligeiramente sujas e desgastadas pela viagem no tempo.");
            Outfit.ClothingPieces = {TEXT("Camisa de campo cáqui"), TEXT("Calças cargo"), TEXT("Botas de caminhada"), TEXT("Chapéu de aba larga"), TEXT("Colete com bolsos")};
            Outfit.bShowsWearAndTear = true;
            break;
            
        case ECharacterArchetype::Survivor:
            Outfit.OutfitName = TEXT("Roupas de Sobrevivência Improvisadas");
            Outfit.Description = TEXT("Mistura de roupas originais com adaptações feitas com materiais encontrados no ambiente pré-histórico.");
            Outfit.ClothingPieces = {TEXT("Camisa rasgada"), TEXT("Calças remendadas"), TEXT("Botas desgastadas"), TEXT("Cinto de couro improvisado"), TEXT("Capa de pele de animal")};
            Outfit.bShowsWearAndTear = true;
            break;
            
        case ECharacterArchetype::Researcher:
            Outfit.OutfitName = TEXT("Roupas Acadêmicas Adaptadas");
            Outfit.Description = TEXT("Roupas formais que foram adaptadas para a vida selvagem, mantendo alguns elementos do mundo moderno.");
            Outfit.ClothingPieces = {TEXT("Camisa social adaptada"), TEXT("Calças de tecido resistente"), TEXT("Sapatos de couro"), TEXT("Óculos"), TEXT("Mochila de pesquisa")};
            Outfit.bShowsWearAndTear = false;
            break;
            
        case ECharacterArchetype::Explorer:
            Outfit.OutfitName = TEXT("Equipamento de Exploração");
            Outfit.Description = TEXT("Roupas técnicas e resistentes, ideais para exploração em ambiente hostil.");
            Outfit.ClothingPieces = {TEXT("Jaqueta técnica"), TEXT("Calças de trilha"), TEXT("Botas de escalada"), TEXT("Luvas"), TEXT("Equipamentos de escalada")};
            Outfit.bShowsWearAndTear = false;
            break;
            
        case ECharacterArchetype::Tribal:
            Outfit.OutfitName = TEXT("Roupas Tribais Adaptadas");
            Outfit.Description = TEXT("Mistura de roupas modernas com elementos tribais criados no mundo pré-histórico.");
            Outfit.ClothingPieces = {TEXT("Túnica de pele"), TEXT("Calças de couro"), TEXT("Sandálias"), TEXT("Adornos tribais"), TEXT("Pintura corporal")};
            Outfit.bShowsWearAndTear = true;
            break;
            
        case ECharacterArchetype::Wanderer:
            Outfit.OutfitName = TEXT("Roupas de Andarilho");
            Outfit.Description = TEXT("Roupas práticas e resistentes, com sinais de longa jornada.");
            Outfit.ClothingPieces = {TEXT("Casaco de viagem"), TEXT("Calças resistentes"), TEXT("Botas de caminhada"), TEXT("Mochila grande"), TEXT("Bastão de caminhada")};
            Outfit.bShowsWearAndTear = true;
            break;
    }
    
    Outfit.bIsWeatherAppropriate = true;
    
    return Outfit;
}